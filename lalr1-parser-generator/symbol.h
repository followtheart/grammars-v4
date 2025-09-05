#pragma once

#include "token.h"
#include <string>
#include <memory>
#include <set>

namespace lalr1 {

// Symbol types
enum class SymbolType {
    TERMINAL,       // Terminal symbol (token)
    NONTERMINAL,    // Non-terminal symbol
    EPSILON,        // Empty string/epsilon
    END_OF_INPUT    // $ symbol
};

// Grammar symbol
class Symbol {
public:
    Symbol(const std::string& name, SymbolType type);
    Symbol(const std::string& name, TokenType token_type);  // For terminals
    
    const std::string& name() const { return name_; }
    SymbolType type() const { return type_; }
    TokenType token_type() const { return token_type_; }
    
    bool is_terminal() const { return type_ == SymbolType::TERMINAL; }
    bool is_nonterminal() const { return type_ == SymbolType::NONTERMINAL; }
    bool is_epsilon() const { return type_ == SymbolType::EPSILON; }
    bool is_end_of_input() const { return type_ == SymbolType::END_OF_INPUT; }
    
    // Comparison operators for use in containers
    bool operator==(const Symbol& other) const;
    bool operator!=(const Symbol& other) const;
    bool operator<(const Symbol& other) const;
    
    std::string to_string() const;

private:
    std::string name_;
    SymbolType type_;
    TokenType token_type_;  // Only used for terminals
};

using SymbolPtr = std::shared_ptr<Symbol>;

// Symbol table for managing symbols
class SymbolTable {
public:
    SymbolTable();
    ~SymbolTable();
    
    // Get or create symbols
    SymbolPtr get_terminal(const std::string& name, TokenType token_type);
    SymbolPtr get_nonterminal(const std::string& name);
    SymbolPtr get_epsilon();
    SymbolPtr get_end_of_input();
    
    // Lookup existing symbols
    SymbolPtr find_symbol(const std::string& name) const;
    
    // Get all symbols of a type
    std::set<SymbolPtr> get_terminals() const;
    std::set<SymbolPtr> get_nonterminals() const;
    std::set<SymbolPtr> get_all_symbols() const;
    
    // Clear all symbols
    void clear();
    
    // Print symbol table
    void print_symbols() const;

private:
    std::set<SymbolPtr> symbols_;
    SymbolPtr epsilon_;
    SymbolPtr end_of_input_;
};

} // namespace lalr1