#pragma once

#include "symbol.h"
#include <vector>
#include <memory>
#include <string>
#include <set>
#include <unordered_map>

namespace lalr1 {

// Production rule: A -> α
class Production {
public:
    Production(SymbolPtr lhs);
    Production(SymbolPtr lhs, const std::vector<SymbolPtr>& rhs);
    
    SymbolPtr lhs() const { return lhs_; }
    const std::vector<SymbolPtr>& rhs() const { return rhs_; }
    
    void add_symbol(SymbolPtr symbol);
    void clear_rhs();
    
    bool is_epsilon_production() const;
    size_t length() const { return rhs_.size(); }
    
    std::string to_string() const;
    
    // Comparison operators
    bool operator==(const Production& other) const;
    bool operator!=(const Production& other) const;
    bool operator<(const Production& other) const;

private:
    SymbolPtr lhs_;
    std::vector<SymbolPtr> rhs_;
};

using ProductionPtr = std::shared_ptr<Production>;

// Context-free grammar
class Grammar {
public:
    Grammar();
    ~Grammar();
    
    // Symbol management
    SymbolTable& symbol_table() { return symbol_table_; }
    const SymbolTable& symbol_table() const { return symbol_table_; }
    
    // Production management
    ProductionPtr add_production(SymbolPtr lhs);
    ProductionPtr add_production(SymbolPtr lhs, const std::vector<SymbolPtr>& rhs);
    
    const std::vector<ProductionPtr>& productions() const { return productions_; }
    std::vector<ProductionPtr> productions_for(SymbolPtr symbol) const;
    
    // Start symbol
    void set_start_symbol(SymbolPtr start) { start_symbol_ = start; }
    SymbolPtr start_symbol() const { return start_symbol_; }
    
    // Grammar analysis
    std::set<SymbolPtr> first_set(SymbolPtr symbol);
    std::set<SymbolPtr> first_set(const std::vector<SymbolPtr>& symbols);
    std::set<SymbolPtr> follow_set(SymbolPtr symbol);
    
    // Const versions
    std::set<SymbolPtr> first_set(SymbolPtr symbol) const;
    std::set<SymbolPtr> first_set(const std::vector<SymbolPtr>& symbols) const;
    std::set<SymbolPtr> follow_set(SymbolPtr symbol) const;
    
    // Check if symbol derives epsilon
    bool derives_epsilon(SymbolPtr symbol);
    bool derives_epsilon(const std::vector<SymbolPtr>& symbols);
    
    // Const versions
    bool derives_epsilon(SymbolPtr symbol) const;
    bool derives_epsilon(const std::vector<SymbolPtr>& symbols) const;
    
    // Validate grammar
    bool is_valid() const;
    std::vector<std::string> get_validation_errors() const;
    
    // Augment grammar with new start symbol (for LALR(1))
    void augment();
    bool is_augmented() const { return is_augmented_; }
    
    // Display
    void print_grammar() const;
    void print_first_sets();
    void print_follow_sets();

private:
    SymbolTable symbol_table_;
    std::vector<ProductionPtr> productions_;
    SymbolPtr start_symbol_;
    bool is_augmented_;
    
    // Cached first/follow sets
    mutable std::unordered_map<SymbolPtr, std::set<SymbolPtr>> first_sets_;
    mutable std::unordered_map<SymbolPtr, std::set<SymbolPtr>> follow_sets_;
    mutable std::unordered_map<SymbolPtr, bool> epsilon_derivers_;
    mutable bool first_sets_computed_;
    mutable bool follow_sets_computed_;
    mutable bool epsilon_computed_;
    
    // Helper methods
    void compute_first_sets();
    void compute_follow_sets();
    void compute_epsilon_derivers();
    void clear_cache();
};

} // namespace lalr1