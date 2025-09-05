#pragma once

#include "grammar.h"
#include "lr_items.h"
#include <vector>
#include <map>

namespace lalr1 {

// Action types for parsing table
enum class ActionType {
    SHIFT,      // Shift symbol and go to state
    REDUCE,     // Reduce by production
    ACCEPT,     // Accept input
    ERROR       // Error state
};

// Parsing action
struct Action {
    ActionType type;
    int value;  // State number for SHIFT, production number for REDUCE
    
    Action() : type(ActionType::ERROR), value(-1) {}
    Action(ActionType t, int v = -1) : type(t), value(v) {}
    
    bool is_shift() const { return type == ActionType::SHIFT; }
    bool is_reduce() const { return type == ActionType::REDUCE; }
    bool is_accept() const { return type == ActionType::ACCEPT; }
    bool is_error() const { return type == ActionType::ERROR; }
    
    std::string to_string() const;
};

// LALR(1) parsing table
class ParseTable {
public:
    ParseTable(int num_states, const std::set<SymbolPtr>& terminals, const std::set<SymbolPtr>& nonterminals);
    
    // Set action for state and terminal
    void set_action(int state, SymbolPtr terminal, const Action& action);
    
    // Set goto for state and nonterminal
    void set_goto(int state, SymbolPtr nonterminal, int target_state);
    
    // Get action for state and terminal
    Action get_action(int state, SymbolPtr terminal) const;
    
    // Get goto for state and nonterminal
    int get_goto(int state, SymbolPtr nonterminal) const;
    
    // Check for conflicts
    bool has_conflicts() const;
    std::vector<std::string> get_conflicts() const;
    
    // Print table
    void print_table() const;
    void print_conflicts() const;

private:
    int num_states_;
    std::set<SymbolPtr> terminals_;
    std::set<SymbolPtr> nonterminals_;
    
    // action[state][terminal] = action
    std::map<std::pair<int, SymbolPtr>, Action> action_table_;
    
    // goto[state][nonterminal] = state
    std::map<std::pair<int, SymbolPtr>, int> goto_table_;
    
    mutable std::vector<std::string> conflicts_;
};

// LALR(1) table generator
class LALR1Generator {
public:
    LALR1Generator(const Grammar& grammar);
    
    // Generate LALR(1) parsing table
    std::unique_ptr<ParseTable> generate_table();
    
    // Get generated LALR states
    const std::vector<LALRStatePtr>& states() const { return states_; }
    
    // Print debug information
    void print_states() const;
    void print_first_sets() const;
    void print_follow_sets() const;

private:
    const Grammar& grammar_;
    std::vector<LALRStatePtr> states_;
    std::map<std::pair<int, SymbolPtr>, int> transitions_;
    
    // LALR(1) construction
    void build_lr0_automaton();
    void compute_lalr_lookaheads();
    void build_lalr_states();
    
    // Helper methods
    LR1ItemSet closure(const LR1ItemSet& items);
    LR1ItemSet goto_set(const LR1ItemSet& items, SymbolPtr symbol);
    
    // Lookahead computation
    std::set<SymbolPtr> compute_lookahead(const LR1Item& item, const std::vector<SymbolPtr>& beta);
    
    LR0Automaton lr0_automaton_;
};

} // namespace lalr1