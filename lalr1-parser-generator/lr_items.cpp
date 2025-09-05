#include "lr_items.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <map>

namespace lalr1 {

// LR0Item implementation
LR0Item::LR0Item(ProductionPtr production, size_t dot_position)
    : production_(production), dot_position_(dot_position) {
}

SymbolPtr LR0Item::next_symbol() const {
    if (is_complete()) return nullptr;
    return production_->rhs()[dot_position_];
}

bool LR0Item::is_complete() const {
    return dot_position_ >= production_->rhs().size();
}

LR0Item LR0Item::advance() const {
    return LR0Item(production_, dot_position_ + 1);
}

std::string LR0Item::to_string() const {
    std::ostringstream oss;
    oss << "[" << production_->lhs()->name() << " ->";
    
    for (size_t i = 0; i < production_->rhs().size(); ++i) {
        if (i == dot_position_) {
            oss << " •";
        }
        oss << " " << production_->rhs()[i]->name();
    }
    
    if (is_complete()) {
        oss << " •";
    }
    
    oss << "]";
    return oss.str();
}

bool LR0Item::operator==(const LR0Item& other) const {
    return production_ == other.production_ && dot_position_ == other.dot_position_;
}

bool LR0Item::operator!=(const LR0Item& other) const {
    return !(*this == other);
}

bool LR0Item::operator<(const LR0Item& other) const {
    if (production_ != other.production_) {
        return production_ < other.production_;
    }
    return dot_position_ < other.dot_position_;
}

// LR1Item implementation
LR1Item::LR1Item(ProductionPtr production, size_t dot_position, SymbolPtr lookahead)
    : production_(production), dot_position_(dot_position), lookahead_(lookahead) {
}

LR1Item::LR1Item(const LR0Item& lr0_item, SymbolPtr lookahead)
    : production_(lr0_item.production()), dot_position_(lr0_item.dot_position()), lookahead_(lookahead) {
}

SymbolPtr LR1Item::next_symbol() const {
    if (is_complete()) return nullptr;
    return production_->rhs()[dot_position_];
}

bool LR1Item::is_complete() const {
    return dot_position_ >= production_->rhs().size();
}

LR1Item LR1Item::advance() const {
    return LR1Item(production_, dot_position_ + 1, lookahead_);
}

std::string LR1Item::to_string() const {
    std::ostringstream oss;
    oss << "[" << production_->lhs()->name() << " ->";
    
    for (size_t i = 0; i < production_->rhs().size(); ++i) {
        if (i == dot_position_) {
            oss << " •";
        }
        oss << " " << production_->rhs()[i]->name();
    }
    
    if (is_complete()) {
        oss << " •";
    }
    
    oss << ", " << lookahead_->name() << "]";
    return oss.str();
}

bool LR1Item::operator==(const LR1Item& other) const {
    return production_ == other.production_ && 
           dot_position_ == other.dot_position_ && 
           lookahead_ == other.lookahead_;
}

bool LR1Item::operator!=(const LR1Item& other) const {
    return !(*this == other);
}

bool LR1Item::operator<(const LR1Item& other) const {
    if (production_ != other.production_) {
        return production_ < other.production_;
    }
    if (dot_position_ != other.dot_position_) {
        return dot_position_ < other.dot_position_;
    }
    return lookahead_ < other.lookahead_;
}

// LR0State implementation
LR0State::LR0State(int id, const LR0ItemSet& items) : id_(id), items_(items) {
}

bool LR0State::is_accepting() const {
    // Check if any item is the augmented start production S' -> S • with dot at end
    for (const auto& item : items_) {
        if (item.is_complete() && item.production()->lhs()->name().back() == '\'') {
            // This is the augmented start production
            return true;
        }
    }
    return false;
}

std::set<SymbolPtr> LR0State::get_transition_symbols() const {
    std::set<SymbolPtr> symbols;
    for (const auto& item : items_) {
        auto next_sym = item.next_symbol();
        if (next_sym) {
            symbols.insert(next_sym);
        }
    }
    return symbols;
}

LR0ItemSet LR0State::get_items_for_symbol(SymbolPtr symbol) const {
    LR0ItemSet result;
    for (const auto& item : items_) {
        if (item.next_symbol() == symbol) {
            result.insert(item);
        }
    }
    return result;
}

std::string LR0State::to_string() const {
    std::ostringstream oss;
    oss << "State " << id_ << ":\n";
    for (const auto& item : items_) {
        oss << "  " << item.to_string() << "\n";
    }
    return oss.str();
}

bool LR0State::operator==(const LR0State& other) const {
    return items_ == other.items_;
}

bool LR0State::operator!=(const LR0State& other) const {
    return !(*this == other);
}

bool LR0State::operator<(const LR0State& other) const {
    return items_ < other.items_;
}

// LALRState implementation
LALRState::LALRState(int id, const LR0ItemSet& core) : id_(id), core_(core) {
}

void LALRState::add_lookahead(const LR0Item& item, SymbolPtr lookahead) {
    lookaheads_[item].insert(lookahead);
    // Update items set
    items_.clear();
    for (const auto& core_item : core_) {
        for (const auto& la : lookaheads_[core_item]) {
            items_.insert(LR1Item(core_item, la));
        }
    }
}

bool LALRState::is_accepting() const {
    // Check if any LR(1) item is the augmented start production S' -> S • with dot at end
    for (const auto& item : items_) {
        if (item.is_complete() && item.production()->lhs()->name().back() == '\'') {
            return true;
        }
    }
    return false;
}

std::set<SymbolPtr> LALRState::get_transition_symbols() const {
    std::set<SymbolPtr> symbols;
    for (const auto& item : items_) {
        auto next_sym = item.next_symbol();
        if (next_sym) {
            symbols.insert(next_sym);
        }
    }
    return symbols;
}

LR1ItemSet LALRState::get_items_for_symbol(SymbolPtr symbol) const {
    LR1ItemSet result;
    for (const auto& item : items_) {
        if (item.next_symbol() == symbol) {
            result.insert(item);
        }
    }
    return result;
}

std::string LALRState::to_string() const {
    std::ostringstream oss;
    oss << "LALR State " << id_ << ":\n";
    for (const auto& item : items_) {
        oss << "  " << item.to_string() << "\n";
    }
    return oss.str();
}

bool LALRState::operator==(const LALRState& other) const {
    return core_ == other.core_;
}

bool LALRState::operator!=(const LALRState& other) const {
    return !(*this == other);
}

// LR0Automaton implementation
LR0Automaton::LR0Automaton(const Grammar& grammar) {
    // Start with initial item [S' -> •S] where S' is the augmented start symbol
    auto start_production = grammar.productions()[0];  // Should be S' -> S
    LR0Item initial_item(start_production, 0);
    LR0ItemSet initial_set = {initial_item};
    
    // Compute closure of initial set
    initial_set = closure(initial_set, grammar);
    
    // Create initial state
    auto initial_state = std::make_shared<LR0State>(0, initial_set);
    states_.push_back(initial_state);
    
    // Queue of states to process
    std::vector<LR0StatePtr> work_queue;
    work_queue.push_back(initial_state);
    
    while (!work_queue.empty()) {
        auto current_state = work_queue.back();
        work_queue.pop_back();
        
        // For each symbol that can transition from current state
        for (auto symbol : current_state->get_transition_symbols()) {
            // Compute GOTO(current_state, symbol)
            auto items_for_symbol = current_state->get_items_for_symbol(symbol);
            LR0ItemSet goto_items;
            
            for (const auto& item : items_for_symbol) {
                goto_items.insert(item.advance());
            }
            
            goto_items = closure(goto_items, grammar);
            
            // Check if this state already exists
            LR0StatePtr target_state = nullptr;
            for (const auto& existing_state : states_) {
                if (existing_state->items() == goto_items) {
                    target_state = existing_state;
                    break;
                }
            }
            
            // If new state, create it
            if (!target_state) {
                target_state = std::make_shared<LR0State>(states_.size(), goto_items);
                states_.push_back(target_state);
                work_queue.push_back(target_state);
            }
            
            // Add transition
            transitions_[{current_state->id(), symbol}] = target_state->id();
        }
    }
}

LR0StatePtr LR0Automaton::get_state(int id) const {
    if (id < 0 || id >= static_cast<int>(states_.size())) {
        return nullptr;
    }
    return states_[id];
}

int LR0Automaton::get_transition(int from_state, SymbolPtr symbol) const {
    auto it = transitions_.find({from_state, symbol});
    if (it != transitions_.end()) {
        return it->second;
    }
    return -1;  // No transition
}

LR0ItemSet LR0Automaton::closure(const LR0ItemSet& items, const Grammar& grammar) {
    LR0ItemSet result = items;
    bool changed = true;
    
    while (changed) {
        changed = false;
        LR0ItemSet new_items;
        
        for (const auto& item : result) {
            auto next_sym = item.next_symbol();
            if (next_sym && next_sym->is_nonterminal()) {
                // Add all productions for this nonterminal
                auto productions = grammar.productions_for(next_sym);
                for (const auto& prod : productions) {
                    LR0Item new_item(prod, 0);
                    if (result.find(new_item) == result.end() && 
                        new_items.find(new_item) == new_items.end()) {
                        new_items.insert(new_item);
                        changed = true;
                    }
                }
            }
        }
        
        result.insert(new_items.begin(), new_items.end());
    }
    
    return result;
}

LR0ItemSet LR0Automaton::goto_set(const LR0ItemSet& items, SymbolPtr symbol, const Grammar& grammar) {
    LR0ItemSet result;
    
    for (const auto& item : items) {
        if (item.next_symbol() == symbol) {
            result.insert(item.advance());
        }
    }
    
    return closure(result, grammar);
}

void LR0Automaton::print_automaton() const {
    std::cout << "LR(0) Automaton:\n";
    for (const auto& state : states_) {
        std::cout << state->to_string() << "\n";
    }
    
    std::cout << "Transitions:\n";
    for (const auto& trans : transitions_) {
        std::cout << "  " << trans.first.first << " --" << trans.first.second->name() 
                  << "--> " << trans.second << "\n";
    }
}

} // namespace lalr1