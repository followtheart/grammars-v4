#pragma once

#include "parse_table.h"
#include "lexer.h"
#include <stack>
#include <memory>

namespace lalr1 {

// Parse tree node
class ParseNode {
public:
    ParseNode(SymbolPtr symbol, const std::string& value = "");
    virtual ~ParseNode() = default;
    
    SymbolPtr symbol() const { return symbol_; }
    const std::string& value() const { return value_; }
    const std::vector<std::shared_ptr<ParseNode>>& children() const { return children_; }
    
    void add_child(std::shared_ptr<ParseNode> child);
    void set_value(const std::string& value) { value_ = value; }
    
    bool is_terminal() const { return symbol_->is_terminal(); }
    bool is_nonterminal() const { return symbol_->is_nonterminal(); }
    
    std::string to_string(int indent = 0) const;
    
private:
    SymbolPtr symbol_;
    std::string value_;
    std::vector<std::shared_ptr<ParseNode>> children_;
};

using ParseNodePtr = std::shared_ptr<ParseNode>;

// Parse result
struct ParseResult {
    bool success;
    ParseNodePtr tree;
    std::string error_message;
    size_t error_line;
    size_t error_column;
    
    ParseResult() : success(false), error_line(0), error_column(0) {}
    ParseResult(ParseNodePtr t) : success(true), tree(t), error_line(0), error_column(0) {}
    ParseResult(const std::string& error, size_t line, size_t col) 
        : success(false), error_message(error), error_line(line), error_column(col) {}
};

// LALR(1) parser
class LALR1Parser {
public:
    LALR1Parser(const Grammar& grammar, std::unique_ptr<ParseTable> table);
    
    // Parse input string
    ParseResult parse(const std::string& input);
    
    // Parse with custom lexer
    ParseResult parse(std::unique_ptr<Lexer> lexer);
    
    // Get grammar and table
    const Grammar& grammar() const { return grammar_; }
    const ParseTable& table() const { return *table_; }
    
    // Enable/disable debug output
    void set_debug(bool debug) { debug_ = debug; }
    bool debug() const { return debug_; }

private:
    const Grammar& grammar_;
    std::unique_ptr<ParseTable> table_;
    bool debug_;
    
    // Parsing stack element
    struct StackElement {
        int state;
        SymbolPtr symbol;
        ParseNodePtr node;
        
        StackElement(int s, SymbolPtr sym, ParseNodePtr n = nullptr) 
            : state(s), symbol(sym), node(n) {}
    };
    
    // Parse with given lexer and token stream
    ParseResult parse_internal(std::unique_ptr<Lexer> lexer);
    
    // Error recovery
    std::string format_error(const Token& token, const std::string& message);
    std::set<SymbolPtr> get_expected_symbols(int state);
};

// Factory function to create parser from grammar
std::unique_ptr<LALR1Parser> create_lalr1_parser(const Grammar& grammar);

// Factory function to create parser for calculator expressions
std::unique_ptr<LALR1Parser> create_calculator_parser();

} // namespace lalr1