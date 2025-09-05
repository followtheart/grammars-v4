#pragma once

#include "grammar.h"
#include <set>
#include <map>
#include <unordered_set>

namespace lalr1 {

// LR(0) item: [A -> α•β]
class LR0Item {
public:
    LR0Item(ProductionPtr production, size_t dot_position = 0);
    
    ProductionPtr production() const { return production_; }
    size_t dot_position() const { return dot_position_; }
    
    // Get symbol after dot (nullptr if at end)
    SymbolPtr next_symbol() const;
    
    // Check if dot is at end
    bool is_complete() const;
    
    // Advance dot position
    LR0Item advance() const;
    
    // String representation
    std::string to_string() const;
    
    // Comparison operators
    bool operator==(const LR0Item& other) const;
    bool operator!=(const LR0Item& other) const;
    bool operator<(const LR0Item& other) const;

private:
    ProductionPtr production_;
    size_t dot_position_;
};

// LR(1) item: [A -> α•β, a] where a is lookahead
class LR1Item {
public:
    LR1Item(ProductionPtr production, size_t dot_position, SymbolPtr lookahead);
    LR1Item(const LR0Item& lr0_item, SymbolPtr lookahead);
    
    ProductionPtr production() const { return production_; }
    size_t dot_position() const { return dot_position_; }
    SymbolPtr lookahead() const { return lookahead_; }
    
    // Get the LR(0) core
    LR0Item core() const { return LR0Item(production_, dot_position_); }
    
    // Get symbol after dot (nullptr if at end)
    SymbolPtr next_symbol() const;
    
    // Check if dot is at end
    bool is_complete() const;
    
    // Advance dot position
    LR1Item advance() const;
    
    // String representation
    std::string to_string() const;
    
    // Comparison operators
    bool operator==(const LR1Item& other) const;
    bool operator!=(const LR1Item& other) const;
    bool operator<(const LR1Item& other) const;

private:
    ProductionPtr production_;
    size_t dot_position_;
    SymbolPtr lookahead_;
};

// Set of LR(0) items
using LR0ItemSet = std::set<LR0Item>;

// Set of LR(1) items  
using LR1ItemSet = std::set<LR1Item>;

// LR(0) state
class LR0State {
public:
    LR0State(int id, const LR0ItemSet& items);
    
    int id() const { return id_; }
    const LR0ItemSet& items() const { return items_; }
    
    // Check if this is an accepting state
    bool is_accepting() const;
    
    // Get all symbols that can transition from this state
    std::set<SymbolPtr> get_transition_symbols() const;
    
    // Get items that have given symbol after dot
    LR0ItemSet get_items_for_symbol(SymbolPtr symbol) const;
    
    std::string to_string() const;
    
    bool operator==(const LR0State& other) const;
    bool operator!=(const LR0State& other) const;
    bool operator<(const LR0State& other) const;

private:
    int id_;
    LR0ItemSet items_;
};

using LR0StatePtr = std::shared_ptr<LR0State>;

// LALR(1) state (combines LR(0) core with lookaheads)
class LALRState {
public:
    LALRState(int id, const LR0ItemSet& core);
    
    int id() const { return id_; }
    const LR0ItemSet& core() const { return core_; }
    const LR1ItemSet& items() const { return items_; }
    
    // Add lookahead to existing item or create new LR(1) item
    void add_lookahead(const LR0Item& item, SymbolPtr lookahead);
    
    // Check if this is an accepting state
    bool is_accepting() const;
    
    // Get all symbols that can transition from this state
    std::set<SymbolPtr> get_transition_symbols() const;
    
    // Get LR(1) items that have given symbol after dot
    LR1ItemSet get_items_for_symbol(SymbolPtr symbol) const;
    
    std::string to_string() const;
    
    bool operator==(const LALRState& other) const;
    bool operator!=(const LALRState& other) const;

private:
    int id_;
    LR0ItemSet core_;
    LR1ItemSet items_;
    std::map<LR0Item, std::set<SymbolPtr>> lookaheads_;
};

using LALRStatePtr = std::shared_ptr<LALRState>;

// LR(0) automaton construction
class LR0Automaton {
public:
    LR0Automaton(const Grammar& grammar);
    
    const std::vector<LR0StatePtr>& states() const { return states_; }
    const std::map<std::pair<int, SymbolPtr>, int>& transitions() const { return transitions_; }
    
    // Get state by ID
    LR0StatePtr get_state(int id) const;
    
    // Get transition destination
    int get_transition(int from_state, SymbolPtr symbol) const;
    
    void print_automaton() const;

private:
    std::vector<LR0StatePtr> states_;
    std::map<std::pair<int, SymbolPtr>, int> transitions_;
    
    // Helper methods
    LR0ItemSet closure(const LR0ItemSet& items, const Grammar& grammar);
    LR0ItemSet goto_set(const LR0ItemSet& items, SymbolPtr symbol, const Grammar& grammar);
};

} // namespace lalr1