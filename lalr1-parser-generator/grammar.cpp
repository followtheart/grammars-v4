#include "grammar.h"
#include <iostream>
#include <algorithm>
#include <sstream>

namespace lalr1 {

Production::Production(SymbolPtr lhs) : lhs_(lhs) {
}

Production::Production(SymbolPtr lhs, const std::vector<SymbolPtr>& rhs)
    : lhs_(lhs), rhs_(rhs) {
}

void Production::add_symbol(SymbolPtr symbol) {
    rhs_.push_back(symbol);
}

void Production::clear_rhs() {
    rhs_.clear();
}

bool Production::is_epsilon_production() const {
    return rhs_.size() == 1 && rhs_[0]->is_epsilon();
}

std::string Production::to_string() const {
    std::ostringstream oss;
    oss << lhs_->name() << " ->";
    if (rhs_.empty()) {
        oss << " ε";
    } else {
        for (const auto& symbol : rhs_) {
            oss << " " << symbol->name();
        }
    }
    return oss.str();
}

bool Production::operator==(const Production& other) const {
    if (lhs_ != other.lhs_ || rhs_.size() != other.rhs_.size()) {
        return false;
    }
    
    for (size_t i = 0; i < rhs_.size(); ++i) {
        if (rhs_[i] != other.rhs_[i]) {
            return false;
        }
    }
    return true;
}

bool Production::operator!=(const Production& other) const {
    return !(*this == other);
}

bool Production::operator<(const Production& other) const {
    if (lhs_ != other.lhs_) {
        return lhs_ < other.lhs_;
    }
    return rhs_ < other.rhs_;
}

Grammar::Grammar() : is_augmented_(false), first_sets_computed_(false), 
                     follow_sets_computed_(false), epsilon_computed_(false) {
}

Grammar::~Grammar() {
}

ProductionPtr Grammar::add_production(SymbolPtr lhs) {
    auto prod = std::make_shared<Production>(lhs);
    productions_.push_back(prod);
    clear_cache();
    return prod;
}

ProductionPtr Grammar::add_production(SymbolPtr lhs, const std::vector<SymbolPtr>& rhs) {
    auto prod = std::make_shared<Production>(lhs, rhs);
    productions_.push_back(prod);
    clear_cache();
    return prod;
}

std::vector<ProductionPtr> Grammar::productions_for(SymbolPtr symbol) const {
    std::vector<ProductionPtr> result;
    for (const auto& prod : productions_) {
        if (prod->lhs() == symbol) {
            result.push_back(prod);
        }
    }
    return result;
}

void Grammar::augment() {
    if (is_augmented_ || !start_symbol_) {
        return;
    }
    
    // Create new start symbol S'
    auto new_start = symbol_table_.get_nonterminal(start_symbol_->name() + "'");
    
    // Add production S' -> S
    auto augment_prod = add_production(new_start, {start_symbol_});
    
    // Move augmented production to front
    productions_.erase(std::find(productions_.begin(), productions_.end(), augment_prod));
    productions_.insert(productions_.begin(), augment_prod);
    
    start_symbol_ = new_start;
    is_augmented_ = true;
    clear_cache();
}

std::set<SymbolPtr> Grammar::first_set(SymbolPtr symbol) {
    if (!first_sets_computed_) {
        compute_first_sets();
    }
    
    auto it = first_sets_.find(symbol);
    if (it != first_sets_.end()) {
        return it->second;
    }
    
    return {};
}

std::set<SymbolPtr> Grammar::first_set(const std::vector<SymbolPtr>& symbols) {
    std::set<SymbolPtr> result;
    
    for (size_t i = 0; i < symbols.size(); ++i) {
        auto first_i = first_set(symbols[i]);
        
        // Add all non-epsilon symbols from FIRST(symbols[i])
        for (const auto& sym : first_i) {
            if (!sym->is_epsilon()) {
                result.insert(sym);
            }
        }
        
        // If symbols[i] doesn't derive epsilon, stop here
        if (!derives_epsilon(symbols[i])) {
            break;
        }
        
        // If this is the last symbol and it derives epsilon, add epsilon to result
        if (i == symbols.size() - 1 && derives_epsilon(symbols[i])) {
            result.insert(symbol_table_.get_epsilon());
        }
    }
    
    // If all symbols derive epsilon, add epsilon
    if (symbols.empty() || derives_epsilon(symbols)) {
        result.insert(symbol_table_.get_epsilon());
    }
    
    return result;
}

std::set<SymbolPtr> Grammar::follow_set(SymbolPtr symbol) {
    if (!follow_sets_computed_) {
        compute_follow_sets();
    }
    
    auto it = follow_sets_.find(symbol);
    if (it != follow_sets_.end()) {
        return it->second;
    }
    
    return {};
}

bool Grammar::derives_epsilon(SymbolPtr symbol) {
    if (!epsilon_computed_) {
        compute_epsilon_derivers();
    }
    
    auto it = epsilon_derivers_.find(symbol);
    return it != epsilon_derivers_.end() && it->second;
}

bool Grammar::derives_epsilon(const std::vector<SymbolPtr>& symbols) {
    for (const auto& symbol : symbols) {
        if (!derives_epsilon(symbol)) {
            return false;
        }
    }
    return true;
}

void Grammar::compute_first_sets() {
    first_sets_.clear();
    
    // Initialize first sets for terminals
    for (const auto& terminal : symbol_table_.get_terminals()) {
        first_sets_[terminal].insert(terminal);
    }
    first_sets_[symbol_table_.get_epsilon()].insert(symbol_table_.get_epsilon());
    first_sets_[symbol_table_.get_end_of_input()].insert(symbol_table_.get_end_of_input());
    
    // Initialize empty first sets for nonterminals
    for (const auto& nonterminal : symbol_table_.get_nonterminals()) {
        first_sets_[nonterminal] = {};
    }
    
    // Iteratively compute first sets
    bool changed = true;
    while (changed) {
        changed = false;
        
        for (const auto& prod : productions_) {
            auto& first_lhs = first_sets_[prod->lhs()];
            size_t old_size = first_lhs.size();
            
            if (prod->rhs().empty()) {
                // A -> ε
                first_lhs.insert(symbol_table_.get_epsilon());
            } else {
                // A -> X₁X₂...Xₖ
                for (size_t i = 0; i < prod->rhs().size(); ++i) {
                    auto symbol = prod->rhs()[i];
                    auto& first_symbol = first_sets_[symbol];
                    
                    // Add FIRST(Xi) - {ε} to FIRST(A)
                    for (const auto& sym : first_symbol) {
                        if (!sym->is_epsilon()) {
                            first_lhs.insert(sym);
                        }
                    }
                    
                    // If Xi doesn't derive ε, stop
                    if (!derives_epsilon(symbol)) {
                        break;
                    }
                    
                    // If all symbols derive ε, add ε to FIRST(A)
                    if (i == prod->rhs().size() - 1) {
                        first_lhs.insert(symbol_table_.get_epsilon());
                    }
                }
            }
            
            if (first_lhs.size() != old_size) {
                changed = true;
            }
        }
    }
    
    first_sets_computed_ = true;
}

void Grammar::compute_follow_sets() {
    if (!first_sets_computed_) {
        compute_first_sets();
    }
    
    follow_sets_.clear();
    
    // Initialize empty follow sets for all nonterminals
    for (const auto& nonterminal : symbol_table_.get_nonterminals()) {
        follow_sets_[nonterminal] = {};
    }
    
    // Add $ to FOLLOW(start_symbol)
    if (start_symbol_) {
        follow_sets_[start_symbol_].insert(symbol_table_.get_end_of_input());
    }
    
    // Iteratively compute follow sets
    bool changed = true;
    while (changed) {
        changed = false;
        
        for (const auto& prod : productions_) {
            // For each production A -> αBβ
            for (size_t i = 0; i < prod->rhs().size(); ++i) {
                auto B = prod->rhs()[i];
                
                if (!B->is_nonterminal()) continue;
                
                auto& follow_B = follow_sets_[B];
                size_t old_size = follow_B.size();
                
                // Get β (symbols after B)
                std::vector<SymbolPtr> beta(prod->rhs().begin() + i + 1, prod->rhs().end());
                
                if (beta.empty()) {
                    // A -> αB, so add FOLLOW(A) to FOLLOW(B)
                    auto& follow_A = follow_sets_[prod->lhs()];
                    for (const auto& sym : follow_A) {
                        follow_B.insert(sym);
                    }
                } else {
                    // A -> αBβ, so add FIRST(β) - {ε} to FOLLOW(B)
                    auto first_beta = first_set(beta);
                    for (const auto& sym : first_beta) {
                        if (!sym->is_epsilon()) {
                            follow_B.insert(sym);
                        }
                    }
                    
                    // If ε ∈ FIRST(β), add FOLLOW(A) to FOLLOW(B)
                    if (derives_epsilon(beta)) {
                        auto& follow_A = follow_sets_[prod->lhs()];
                        for (const auto& sym : follow_A) {
                            follow_B.insert(sym);
                        }
                    }
                }
                
                if (follow_B.size() != old_size) {
                    changed = true;
                }
            }
        }
    }
    
    follow_sets_computed_ = true;
}

void Grammar::compute_epsilon_derivers() {
    epsilon_derivers_.clear();
    
    // Initially, no nonterminal derives epsilon
    for (const auto& nonterminal : symbol_table_.get_nonterminals()) {
        epsilon_derivers_[nonterminal] = false;
    }
    
    // Epsilon always derives epsilon
    epsilon_derivers_[symbol_table_.get_epsilon()] = true;
    
    // Iteratively find epsilon derivers
    bool changed = true;
    while (changed) {
        changed = false;
        
        for (const auto& prod : productions_) {
            if (epsilon_derivers_[prod->lhs()]) {
                continue;  // Already derives epsilon
            }
            
            if (prod->rhs().empty() || 
                (prod->rhs().size() == 1 && prod->rhs()[0]->is_epsilon())) {
                // A -> ε
                epsilon_derivers_[prod->lhs()] = true;
                changed = true;
            } else {
                // A -> X₁X₂...Xₖ, check if all Xi derive epsilon
                bool all_derive_epsilon = true;
                for (const auto& symbol : prod->rhs()) {
                    if (symbol->is_terminal() || symbol->is_end_of_input()) {
                        all_derive_epsilon = false;
                        break;
                    }
                    if (!epsilon_derivers_[symbol]) {
                        all_derive_epsilon = false;
                        break;
                    }
                }
                
                if (all_derive_epsilon) {
                    epsilon_derivers_[prod->lhs()] = true;
                    changed = true;
                }
            }
        }
    }
    
    epsilon_computed_ = true;
}

void Grammar::clear_cache() {
    first_sets_.clear();
    follow_sets_.clear();
    epsilon_derivers_.clear();
    first_sets_computed_ = false;
    follow_sets_computed_ = false;
    epsilon_computed_ = false;
}

bool Grammar::is_valid() const {
    return get_validation_errors().empty();
}

std::vector<std::string> Grammar::get_validation_errors() const {
    std::vector<std::string> errors;
    
    if (!start_symbol_) {
        errors.push_back("No start symbol defined");
    }
    
    if (productions_.empty()) {
        errors.push_back("No productions defined");
    }
    
    // Check for undefined nonterminals
    std::set<SymbolPtr> defined_nonterminals;
    std::set<SymbolPtr> used_nonterminals;
    
    for (const auto& prod : productions_) {
        defined_nonterminals.insert(prod->lhs());
        for (const auto& symbol : prod->rhs()) {
            if (symbol->is_nonterminal()) {
                used_nonterminals.insert(symbol);
            }
        }
    }
    
    for (const auto& symbol : used_nonterminals) {
        if (defined_nonterminals.find(symbol) == defined_nonterminals.end()) {
            errors.push_back("Undefined nonterminal: " + symbol->name());
        }
    }
    
    return errors;
}

void Grammar::print_grammar() const {
    std::cout << "Grammar:\n";
    std::cout << "Start symbol: " << (start_symbol_ ? start_symbol_->name() : "none") << "\n";
    std::cout << "Productions:\n";
    for (size_t i = 0; i < productions_.size(); ++i) {
        std::cout << "  " << i << ": " << productions_[i]->to_string() << "\n";
    }
}

void Grammar::print_first_sets() {
    std::cout << "FIRST sets:\n";
    for (const auto& nonterminal : symbol_table_.get_nonterminals()) {
        auto first = first_set(nonterminal);
        std::cout << "  FIRST(" << nonterminal->name() << ") = {";
        bool first_item = true;
        for (const auto& symbol : first) {
            if (!first_item) std::cout << ", ";
            std::cout << symbol->name();
            first_item = false;
        }
        std::cout << "}\n";
    }
}

void Grammar::print_follow_sets() {
    std::cout << "FOLLOW sets:\n";
    for (const auto& nonterminal : symbol_table_.get_nonterminals()) {
        auto follow = follow_set(nonterminal);
        std::cout << "  FOLLOW(" << nonterminal->name() << ") = {";
        bool first_item = true;
        for (const auto& symbol : follow) {
            if (!first_item) std::cout << ", ";
            std::cout << symbol->name();
            first_item = false;
        }
        std::cout << "}\n";
    }
}

} // namespace lalr1