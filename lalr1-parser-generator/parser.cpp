#include "parser.h"
#include <iostream>
#include <sstream>

namespace lalr1 {

ParseNode::ParseNode(SymbolPtr symbol, const std::string& value) 
    : symbol_(symbol), value_(value) {
}

void ParseNode::add_child(std::shared_ptr<ParseNode> child) {
    children_.push_back(child);
}

std::string ParseNode::to_string(int indent) const {
    std::string result(indent * 2, ' ');
    result += symbol_->name();
    
    if (!value_.empty() && value_ != symbol_->name()) {
        result += " (\"" + value_ + "\")";
    }
    
    if (!children_.empty()) {
        result += "\n";
        for (size_t i = 0; i < children_.size(); ++i) {
            if (i > 0) result += "\n";
            result += children_[i]->to_string(indent + 1);
        }
    }
    
    return result;
}

LALR1Parser::LALR1Parser(const Grammar& grammar, std::unique_ptr<ParseTable> table)
    : grammar_(grammar), table_(std::move(table)), debug_(false) {
}

ParseResult LALR1Parser::parse(const std::string& input) {
    auto lexer = create_calculator_lexer();
    lexer->set_input(input);
    return parse_internal(std::move(lexer));
}

ParseResult LALR1Parser::parse(std::unique_ptr<Lexer> lexer) {
    return parse_internal(std::move(lexer));
}

ParseResult LALR1Parser::parse_internal(std::unique_ptr<Lexer> lexer) {
    std::stack<StackElement> stack;
    
    // Initialize with state 0
    stack.push(StackElement(0, nullptr));
    
    Token current_token = lexer->next_token();
    
    if (debug_) {
        std::cout << "Starting parse...\n";
    }
    
    while (true) {
        int current_state = stack.top().state;
        
        // Find terminal symbol for current token
        SymbolPtr terminal = nullptr;
        if (current_token.is_eof()) {
            terminal = grammar_.symbol_table().get_end_of_input();
        } else {
            // Find terminal with matching token type
            for (const auto& sym : grammar_.symbol_table().get_terminals()) {
                if (sym->is_terminal() && sym->token_type() == current_token.type) {
                    terminal = sym;
                    break;
                }
            }
        }
        
        if (!terminal) {
            return ParseResult("Unknown token: " + current_token.value, 
                             current_token.line, current_token.column);
        }
        
        Action action = table_->get_action(current_state, terminal);
        
        if (debug_) {
            std::cout << "State " << current_state << ", Token " << terminal->name() 
                      << " (\"" << current_token.value << "\"), Action: " << action.to_string() << "\n";
        }
        
        if (action.is_shift()) {
            // Shift: push terminal and new state
            auto node = std::make_shared<ParseNode>(terminal, current_token.value);
            stack.push(StackElement(action.value, terminal, node));
            current_token = lexer->next_token();
            
        } else if (action.is_reduce()) {
            // Reduce: pop RHS symbols and push LHS
            auto production = grammar_.productions()[action.value];
            
            if (debug_) {
                std::cout << "  Reducing by: " << production->to_string() << "\n";
            }
            
            // Create node for LHS
            auto lhs_node = std::make_shared<ParseNode>(production->lhs());
            
            // Pop RHS symbols from stack
            std::vector<ParseNodePtr> rhs_nodes;
            for (size_t i = 0; i < production->rhs().size(); ++i) {
                if (stack.empty()) {
                    return ParseResult("Stack underflow during reduction", 
                                     current_token.line, current_token.column);
                }
                rhs_nodes.push_back(stack.top().node);
                stack.pop();
            }
            
            // Add children in correct order (reverse since we popped from stack)
            for (auto it = rhs_nodes.rbegin(); it != rhs_nodes.rend(); ++it) {
                if (*it) {
                    lhs_node->add_child(*it);
                }
            }
            
            // Get current state after popping RHS
            int state_after_pop = stack.empty() ? 0 : stack.top().state;
            
            // Get goto state for LHS
            int goto_state = table_->get_goto(state_after_pop, production->lhs());
            if (goto_state < 0) {
                return ParseResult("No goto entry for state " + std::to_string(state_after_pop) + 
                                 " and symbol " + production->lhs()->name(),
                                 current_token.line, current_token.column);
            }
            
            // Push LHS and goto state
            stack.push(StackElement(goto_state, production->lhs(), lhs_node));
            
        } else if (action.is_accept()) {
            // Accept: parsing successful
            if (stack.size() >= 2) {
                auto result_node = stack.top().node;
                return ParseResult(result_node);
            } else {
                return ParseResult("Invalid stack state at accept", 
                                 current_token.line, current_token.column);
            }
            
        } else {
            // Error
            std::string expected_symbols;
            auto expected = get_expected_symbols(current_state);
            bool first = true;
            for (const auto& sym : expected) {
                if (!first) expected_symbols += ", ";
                expected_symbols += sym->name();
                first = false;
            }
            
            std::string error = "Unexpected token '" + current_token.value + "'";
            if (!expected_symbols.empty()) {
                error += ". Expected: " + expected_symbols;
            }
            
            return ParseResult(error, current_token.line, current_token.column);
        }
    }
}

std::string LALR1Parser::format_error(const Token& token, const std::string& message) {
    std::ostringstream oss;
    oss << "Parse error at line " << token.line << ", column " << token.column 
        << ": " << message;
    return oss.str();
}

std::set<SymbolPtr> LALR1Parser::get_expected_symbols(int state) {
    std::set<SymbolPtr> expected;
    
    // Get all terminals that have non-error actions in this state
    for (const auto& terminal : grammar_.symbol_table().get_terminals()) {
        auto action = table_->get_action(state, terminal);
        if (!action.is_error()) {
            expected.insert(terminal);
        }
    }
    
    // Also check end of input
    auto eof_action = table_->get_action(state, grammar_.symbol_table().get_end_of_input());
    if (!eof_action.is_error()) {
        expected.insert(grammar_.symbol_table().get_end_of_input());
    }
    
    return expected;
}

std::unique_ptr<LALR1Parser> create_lalr1_parser(const Grammar& grammar) {
    // Make a copy of the grammar and augment it
    Grammar augmented_grammar = grammar;
    augmented_grammar.augment();
    
    // Generate LALR(1) table
    LALR1Generator generator(augmented_grammar);
    auto table = generator.generate_table();
    
    if (table->has_conflicts()) {
        table->print_conflicts();
        throw std::runtime_error("Grammar has parsing conflicts");
    }
    
    return std::make_unique<LALR1Parser>(augmented_grammar, std::move(table));
}

std::unique_ptr<LALR1Parser> create_calculator_parser() {
    Grammar grammar;
    
    // Create symbols
    auto E = grammar.symbol_table().get_nonterminal("E");
    auto T = grammar.symbol_table().get_nonterminal("T");
    auto F = grammar.symbol_table().get_nonterminal("F");
    
    auto plus = grammar.symbol_table().get_terminal("+", TokenType::PLUS);
    auto minus = grammar.symbol_table().get_terminal("-", TokenType::MINUS);
    auto multiply = grammar.symbol_table().get_terminal("*", TokenType::MULTIPLY);
    auto divide = grammar.symbol_table().get_terminal("/", TokenType::DIVIDE);
    auto power = grammar.symbol_table().get_terminal("^", TokenType::POWER);
    auto lparen = grammar.symbol_table().get_terminal("(", TokenType::LPAREN);
    auto rparen = grammar.symbol_table().get_terminal(")", TokenType::RPAREN);
    auto number = grammar.symbol_table().get_terminal("num", TokenType::NUMBER);
    
    // Grammar rules:
    // E -> E + T | E - T | T
    // T -> T * F | T / F | F
    // F -> F ^ F | ( E ) | num
    
    grammar.add_production(E, {E, plus, T});
    grammar.add_production(E, {E, minus, T});
    grammar.add_production(E, {T});
    
    grammar.add_production(T, {T, multiply, F});
    grammar.add_production(T, {T, divide, F});
    grammar.add_production(T, {F});
    
    grammar.add_production(F, {F, power, F});
    grammar.add_production(F, {lparen, E, rparen});
    grammar.add_production(F, {number});
    
    grammar.set_start_symbol(E);
    
    return create_lalr1_parser(grammar);
}

} // namespace lalr1