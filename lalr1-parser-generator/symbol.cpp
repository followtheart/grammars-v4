#include "symbol.h"
#include <iostream>
#include <algorithm>

namespace lalr1 {

Symbol::Symbol(const std::string& name, SymbolType type)
    : name_(name), type_(type), token_type_(TokenType::EOF_TOKEN) {
}

Symbol::Symbol(const std::string& name, TokenType token_type)
    : name_(name), type_(SymbolType::TERMINAL), token_type_(token_type) {
}

bool Symbol::operator==(const Symbol& other) const {
    return name_ == other.name_ && type_ == other.type_ && token_type_ == other.token_type_;
}

bool Symbol::operator!=(const Symbol& other) const {
    return !(*this == other);
}

bool Symbol::operator<(const Symbol& other) const {
    if (name_ != other.name_) {
        return name_ < other.name_;
    }
    if (type_ != other.type_) {
        return type_ < other.type_;
    }
    return token_type_ < other.token_type_;
}

std::string Symbol::to_string() const {
    std::string type_str;
    switch (type_) {
        case SymbolType::TERMINAL: type_str = "T"; break;
        case SymbolType::NONTERMINAL: type_str = "NT"; break;
        case SymbolType::EPSILON: type_str = "ε"; break;
        case SymbolType::END_OF_INPUT: type_str = "$"; break;
    }
    
    if (is_terminal() && token_type_ != TokenType::EOF_TOKEN) {
        return name_ + "[" + type_str + ":" + token_type_to_string(token_type_) + "]";
    } else {
        return name_ + "[" + type_str + "]";
    }
}

SymbolTable::SymbolTable() {
    epsilon_ = std::make_shared<Symbol>("ε", SymbolType::EPSILON);
    end_of_input_ = std::make_shared<Symbol>("$", SymbolType::END_OF_INPUT);
    symbols_.insert(epsilon_);
    symbols_.insert(end_of_input_);
}

SymbolTable::~SymbolTable() {
}

SymbolPtr SymbolTable::get_terminal(const std::string& name, TokenType token_type) {
    // Look for existing terminal with same name and token type
    for (const auto& symbol : symbols_) {
        if (symbol->name() == name && symbol->is_terminal() && symbol->token_type() == token_type) {
            return symbol;
        }
    }
    
    // Create new terminal
    auto symbol = std::make_shared<Symbol>(name, token_type);
    symbols_.insert(symbol);
    return symbol;
}

SymbolPtr SymbolTable::get_nonterminal(const std::string& name) {
    // Look for existing nonterminal with same name
    for (const auto& symbol : symbols_) {
        if (symbol->name() == name && symbol->is_nonterminal()) {
            return symbol;
        }
    }
    
    // Create new nonterminal
    auto symbol = std::make_shared<Symbol>(name, SymbolType::NONTERMINAL);
    symbols_.insert(symbol);
    return symbol;
}

SymbolPtr SymbolTable::get_epsilon() {
    return epsilon_;
}

SymbolPtr SymbolTable::get_end_of_input() {
    return end_of_input_;
}

SymbolPtr SymbolTable::find_symbol(const std::string& name) const {
    for (const auto& symbol : symbols_) {
        if (symbol->name() == name) {
            return symbol;
        }
    }
    return nullptr;
}

std::set<SymbolPtr> SymbolTable::get_terminals() const {
    std::set<SymbolPtr> terminals;
    for (const auto& symbol : symbols_) {
        if (symbol->is_terminal()) {
            terminals.insert(symbol);
        }
    }
    return terminals;
}

std::set<SymbolPtr> SymbolTable::get_nonterminals() const {
    std::set<SymbolPtr> nonterminals;
    for (const auto& symbol : symbols_) {
        if (symbol->is_nonterminal()) {
            nonterminals.insert(symbol);
        }
    }
    return nonterminals;
}

std::set<SymbolPtr> SymbolTable::get_all_symbols() const {
    return symbols_;
}

void SymbolTable::clear() {
    symbols_.clear();
    // Re-add epsilon and end of input
    epsilon_ = std::make_shared<Symbol>("ε", SymbolType::EPSILON);
    end_of_input_ = std::make_shared<Symbol>("$", SymbolType::END_OF_INPUT);
    symbols_.insert(epsilon_);
    symbols_.insert(end_of_input_);
}

void SymbolTable::print_symbols() const {
    std::cout << "Symbol Table:\n";
    std::cout << "Terminals:\n";
    for (const auto& symbol : get_terminals()) {
        std::cout << "  " << symbol->to_string() << "\n";
    }
    
    std::cout << "Nonterminals:\n";
    for (const auto& symbol : get_nonterminals()) {
        std::cout << "  " << symbol->to_string() << "\n";
    }
    
    std::cout << "Special:\n";
    std::cout << "  " << epsilon_->to_string() << "\n";
    std::cout << "  " << end_of_input_->to_string() << "\n";
}

} // namespace lalr1