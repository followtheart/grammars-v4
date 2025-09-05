#include "parse_table.h"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace lalr1 {

std::string Action::to_string() const {
    switch (type) {
        case ActionType::SHIFT:
            return "s" + std::to_string(value);
        case ActionType::REDUCE:
            return "r" + std::to_string(value);
        case ActionType::ACCEPT:
            return "acc";
        case ActionType::ERROR:
            return "err";
    }
    return "?";
}

ParseTable::ParseTable(int num_states, const std::set<SymbolPtr>& terminals, const std::set<SymbolPtr>& nonterminals)
    : num_states_(num_states), terminals_(terminals), nonterminals_(nonterminals) {
}

void ParseTable::set_action(int state, SymbolPtr terminal, const Action& action) {
    auto key = std::make_pair(state, terminal);
    
    // Check for conflicts
    auto existing = action_table_.find(key);
    if (existing != action_table_.end()) {
        if (existing->second.type != action.type || existing->second.value != action.value) {
            std::ostringstream oss;
            oss << "Action conflict in state " << state << " on terminal " << terminal->name()
                << ": existing=" << existing->second.to_string() << ", new=" << action.to_string();
            conflicts_.push_back(oss.str());
        }
    }
    
    action_table_[key] = action;
}

void ParseTable::set_goto(int state, SymbolPtr nonterminal, int target_state) {
    auto key = std::make_pair(state, nonterminal);
    goto_table_[key] = target_state;
}

Action ParseTable::get_action(int state, SymbolPtr terminal) const {
    auto key = std::make_pair(state, terminal);
    auto it = action_table_.find(key);
    if (it != action_table_.end()) {
        return it->second;
    }
    return Action(ActionType::ERROR);
}

int ParseTable::get_goto(int state, SymbolPtr nonterminal) const {
    auto key = std::make_pair(state, nonterminal);
    auto it = goto_table_.find(key);
    if (it != goto_table_.end()) {
        return it->second;
    }
    return -1;
}

bool ParseTable::has_conflicts() const {
    return !conflicts_.empty();
}

std::vector<std::string> ParseTable::get_conflicts() const {
    return conflicts_;
}

void ParseTable::print_table() const {
    std::cout << "\nLALR(1) Parsing Table:\n";
    
    // Print header
    std::cout << std::setw(6) << "State";
    
    // Action columns (terminals)
    std::cout << " | ACTION";
    for (const auto& terminal : terminals_) {
        std::cout << std::setw(8) << terminal->name();
    }
    
    // Goto columns (nonterminals)
    std::cout << " | GOTO";
    for (const auto& nonterminal : nonterminals_) {
        if (nonterminal->name() != "ε" && nonterminal->name() != "$") {
            std::cout << std::setw(8) << nonterminal->name();
        }
    }
    std::cout << "\n";
    
    // Print separator
    std::cout << std::string(6 + 8 + terminals_.size() * 8 + 6 + nonterminals_.size() * 8, '-') << "\n";
    
    // Print rows
    for (int state = 0; state < num_states_; ++state) {
        std::cout << std::setw(6) << state;
        
        // Action columns
        std::cout << " |      ";
        for (const auto& terminal : terminals_) {
            auto action = get_action(state, terminal);
            std::cout << std::setw(8) << action.to_string();
        }
        
        // Goto columns
        std::cout << " |    ";
        for (const auto& nonterminal : nonterminals_) {
            if (nonterminal->name() != "ε" && nonterminal->name() != "$") {
                int goto_state = get_goto(state, nonterminal);
                if (goto_state >= 0) {
                    std::cout << std::setw(8) << goto_state;
                } else {
                    std::cout << std::setw(8) << "";
                }
            }
        }
        std::cout << "\n";
    }
}

void ParseTable::print_conflicts() const {
    if (conflicts_.empty()) {
        std::cout << "No conflicts found.\n";
    } else {
        std::cout << "Parsing conflicts:\n";
        for (const auto& conflict : conflicts_) {
            std::cout << "  " << conflict << "\n";
        }
    }
}

LALR1Generator::LALR1Generator(const Grammar& grammar) : grammar_(grammar), lr0_automaton_(grammar) {
}

std::unique_ptr<ParseTable> LALR1Generator::generate_table() {
    if (!grammar_.is_augmented()) {
        throw std::runtime_error("Grammar must be augmented before generating LALR(1) table");
    }
    
    // Build LALR(1) states
    build_lalr_states();
    
    // Create parse table
    auto terminals = grammar_.symbol_table().get_terminals();
    terminals.insert(grammar_.symbol_table().get_end_of_input());
    auto nonterminals = grammar_.symbol_table().get_nonterminals();
    
    auto table = std::make_unique<ParseTable>(states_.size(), terminals, nonterminals);
    
    // Fill action and goto tables
    for (const auto& state : states_) {
        // For each LR(1) item in the state
        for (const auto& item : state->items()) {
            if (item.is_complete()) {
                // Reduce action
                if (item.production()->lhs()->name().back() == '\'' && 
                    item.production()->rhs().size() == 1) {
                    // This is S' -> S, so accept
                    table->set_action(state->id(), item.lookahead(), Action(ActionType::ACCEPT));
                } else {
                    // Find production number
                    auto productions = grammar_.productions();
                    for (size_t i = 0; i < productions.size(); ++i) {
                        if (productions[i] == item.production()) {
                            table->set_action(state->id(), item.lookahead(), Action(ActionType::REDUCE, i));
                            break;
                        }
                    }
                }
            } else {
                // Shift or goto
                auto next_symbol = item.next_symbol();
                if (next_symbol && next_symbol->is_terminal()) {
                    // Shift action
                    int target_state = lr0_automaton_.get_transition(state->id(), next_symbol);
                    if (target_state >= 0) {
                        table->set_action(state->id(), next_symbol, Action(ActionType::SHIFT, target_state));
                    }
                }
            }
        }
        
        // Goto entries for nonterminals
        for (auto symbol : state->get_transition_symbols()) {
            if (symbol->is_nonterminal()) {
                int target_state = lr0_automaton_.get_transition(state->id(), symbol);
                if (target_state >= 0) {
                    table->set_goto(state->id(), symbol, target_state);
                }
            }
        }
    }
    
    return table;
}

void LALR1Generator::build_lalr_states() {
    // Convert LR(0) states to LALR states with computed lookaheads
    const auto& lr0_states = lr0_automaton_.states();
    
    for (const auto& lr0_state : lr0_states) {
        auto lalr_state = std::make_shared<LALRState>(lr0_state->id(), lr0_state->items());
        
        // Compute lookaheads for each LR(0) item
        for (const auto& lr0_item : lr0_state->items()) {
            // Start with empty lookahead set
            std::set<SymbolPtr> lookaheads;
            
            if (lr0_item.is_complete()) {
                // For complete items, compute FOLLOW set of LHS
                if (lr0_item.production()->lhs()->name().back() == '\'' &&
                    lr0_item.production()->rhs().size() == 1) {
                    // This is S' -> S, add $ as lookahead
                    lookaheads.insert(grammar_.symbol_table().get_end_of_input());
                } else {
                    // Add FOLLOW(A) where A is the LHS of the production
                    auto follow_set = grammar_.follow_set(lr0_item.production()->lhs());
                    lookaheads.insert(follow_set.begin(), follow_set.end());
                }
            } else {
                // For shift items, we need to propagate lookaheads
                // This is a simplified version - in full LALR(1), we'd do proper lookahead propagation
                auto next_symbol = lr0_item.next_symbol();
                if (next_symbol && next_symbol->is_terminal()) {
                    lookaheads.insert(next_symbol);
                } else if (next_symbol && next_symbol->is_nonterminal()) {
                    // Add FIRST set of what follows the nonterminal
                    auto first_set = grammar_.first_set(next_symbol);
                    lookaheads.insert(first_set.begin(), first_set.end());
                }
            }
            
            // Add all computed lookaheads to the LALR state
            for (const auto& lookahead : lookaheads) {
                lalr_state->add_lookahead(lr0_item, lookahead);
            }
        }
        
        states_.push_back(lalr_state);
    }
}

void LALR1Generator::print_states() const {
    std::cout << "\nLALR(1) States:\n";
    for (const auto& state : states_) {
        std::cout << state->to_string() << "\n";
    }
}

void LALR1Generator::print_first_sets() const {
    std::cout << "\nFIRST Sets:\n";
    for (const auto& nonterminal : grammar_.symbol_table().get_nonterminals()) {
        auto first_set = grammar_.first_set(nonterminal);
        std::cout << "FIRST(" << nonterminal->name() << ") = {";
        bool first = true;
        for (const auto& symbol : first_set) {
            if (!first) std::cout << ", ";
            std::cout << symbol->name();
            first = false;
        }
        std::cout << "}\n";
    }
}

void LALR1Generator::print_follow_sets() const {
    std::cout << "\nFOLLOW Sets:\n";
    for (const auto& nonterminal : grammar_.symbol_table().get_nonterminals()) {
        auto follow_set = grammar_.follow_set(nonterminal);
        std::cout << "FOLLOW(" << nonterminal->name() << ") = {";
        bool first = true;
        for (const auto& symbol : follow_set) {
            if (!first) std::cout << ", ";
            std::cout << symbol->name();
            first = false;
        }
        std::cout << "}\n";
    }
}

} // namespace lalr1