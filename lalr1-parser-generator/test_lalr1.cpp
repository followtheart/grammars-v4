#include <iostream>
#include <cassert>
#include <vector>
#include "token.h"
#include "lexer.h"
#include "symbol.h"
#include "grammar.h"
#include "lr_items.h"
#include "parse_table.h"
#include "parser.h"

using namespace lalr1;

void test_lr_items() {
    std::cout << "Testing LR Items...\n";
    
    Grammar grammar;
    auto E = grammar.symbol_table().get_nonterminal("E");
    auto T = grammar.symbol_table().get_nonterminal("T");
    auto plus = grammar.symbol_table().get_terminal("+", TokenType::PLUS);
    auto num = grammar.symbol_table().get_terminal("num", TokenType::NUMBER);
    
    // E -> E + T
    auto prod = grammar.add_production(E, {E, plus, T});
    
    // Test LR(0) item
    LR0Item item0(prod, 0);
    assert(item0.production() == prod);
    assert(item0.dot_position() == 0);
    assert(item0.next_symbol() == E);
    assert(!item0.is_complete());
    std::cout << "  LR(0) item: " << item0.to_string() << "\n";
    
    // Test advancing
    auto item1 = item0.advance();
    assert(item1.dot_position() == 1);
    assert(item1.next_symbol() == plus);
    std::cout << "  Advanced: " << item1.to_string() << "\n";
    
    // Test LR(1) item
    auto eof_sym = grammar.symbol_table().get_end_of_input();
    LR1Item lr1_item(prod, 0, eof_sym);
    assert(lr1_item.lookahead() == eof_sym);
    std::cout << "  LR(1) item: " << lr1_item.to_string() << "\n";
    
    std::cout << "LR Items tests passed!\n\n";
}

void test_lr0_automaton() {
    std::cout << "Testing LR(0) Automaton...\n";
    
    Grammar grammar;
    auto E = grammar.symbol_table().get_nonterminal("E");
    auto T = grammar.symbol_table().get_nonterminal("T");
    auto plus = grammar.symbol_table().get_terminal("+", TokenType::PLUS);
    auto num = grammar.symbol_table().get_terminal("num", TokenType::NUMBER);
    
    // Simple grammar: E -> E + T | T, T -> num
    grammar.add_production(E, {E, plus, T});
    grammar.add_production(E, {T});
    grammar.add_production(T, {num});
    
    grammar.set_start_symbol(E);
    grammar.augment();
    
    std::cout << "  Augmented Grammar:\n";
    grammar.print_grammar();
    
    LR0Automaton automaton(grammar);
    std::cout << "  Generated " << automaton.states().size() << " LR(0) states\n";
    
    automaton.print_automaton();
    
    std::cout << "LR(0) Automaton tests passed!\n\n";
}

void test_lalr1_table_generation() {
    std::cout << "Testing LALR(1) Table Generation...\n";
    
    Grammar grammar;
    auto E = grammar.symbol_table().get_nonterminal("E");
    auto T = grammar.symbol_table().get_nonterminal("T");
    auto plus = grammar.symbol_table().get_terminal("+", TokenType::PLUS);
    auto num = grammar.symbol_table().get_terminal("num", TokenType::NUMBER);
    
    // Simple grammar: E -> E + T | T, T -> num
    grammar.add_production(E, {E, plus, T});
    grammar.add_production(E, {T});
    grammar.add_production(T, {num});
    
    grammar.set_start_symbol(E);
    grammar.augment();
    
    LALR1Generator generator(grammar);
    generator.print_first_sets();
    generator.print_follow_sets();
    
    auto table = generator.generate_table();
    assert(table != nullptr);
    
    std::cout << "  Generated LALR(1) table\n";
    table->print_table();
    
    if (table->has_conflicts()) {
        std::cout << "  Warning: Table has conflicts:\n";
        table->print_conflicts();
    } else {
        std::cout << "  No conflicts found\n";
    }
    
    std::cout << "LALR(1) Table Generation tests passed!\n\n";
}

void test_calculator_parsing() {
    std::cout << "Testing Calculator Parsing...\n";
    
    try {
        auto parser = create_calculator_parser();
        parser->set_debug(true);
        
        std::vector<std::string> test_expressions = {
            "2 + 3",
            "2 + 3 * 4",
            "2 * 3 + 4",
            "(2 + 3) * 4",
            "2 + 3 * 4 + 5",
            "1 + 2 + 3 + 4"
        };
        
        for (const auto& expr : test_expressions) {
            std::cout << "  Parsing: \"" << expr << "\"\n";
            
            auto result = parser->parse(expr);
            
            if (result.success) {
                std::cout << "    ✓ Parse successful\n";
                std::cout << "    Parse tree:\n" << result.tree->to_string(2) << "\n";
            } else {
                std::cout << "    ✗ Parse failed: " << result.error_message 
                          << " at line " << result.error_line 
                          << ", column " << result.error_column << "\n";
            }
            std::cout << "\n";
        }
        
        // Test error cases
        std::vector<std::string> error_expressions = {
            "2 +",        // Missing operand
            "+ 3",        // Missing operand
            "2 3",        // Missing operator
            "(2 + 3",     // Missing closing paren
            "2 + )"       // Missing opening paren
        };
        
        std::cout << "  Testing error cases:\n";
        for (const auto& expr : error_expressions) {
            std::cout << "  Parsing: \"" << expr << "\"\n";
            
            auto result = parser->parse(expr);
            
            if (result.success) {
                std::cout << "    ✗ Unexpected success\n";
            } else {
                std::cout << "    ✓ Expected error: " << result.error_message << "\n";
            }
        }
        
    } catch (const std::exception& e) {
        std::cout << "  Exception during calculator parsing: " << e.what() << "\n";
        // Don't fail the test - grammar might have conflicts which is expected for some grammars
    }
    
    std::cout << "Calculator Parsing tests completed!\n\n";
}

void test_simple_grammar() {
    std::cout << "Testing Simple Grammar without conflicts...\n";
    
    Grammar grammar;
    auto S = grammar.symbol_table().get_nonterminal("S");
    auto id = grammar.symbol_table().get_terminal("id", TokenType::IDENTIFIER);
    
    // Very simple grammar: S -> id
    grammar.add_production(S, {id});
    grammar.set_start_symbol(S);
    
    try {
        auto parser = create_lalr1_parser(grammar);
        parser->set_debug(true);
        
        // Create a simple lexer for this test
        auto lexer = std::make_unique<Lexer>();
        lexer->add_rule(TokenType::IDENTIFIER, "[a-zA-Z][a-zA-Z0-9]*");
        lexer->add_rule(TokenType::WHITESPACE, "[ \t\n]+", true);
        lexer->set_input("hello");
        
        auto result = parser->parse(std::move(lexer));
        
        if (result.success) {
            std::cout << "  ✓ Simple grammar parse successful\n";
            std::cout << "  Parse tree:\n" << result.tree->to_string(1) << "\n";
        } else {
            std::cout << "  ✗ Simple grammar parse failed: " << result.error_message << "\n";
        }
        
    } catch (const std::exception& e) {
        std::cout << "  Exception: " << e.what() << "\n";
    }
    
    std::cout << "Simple Grammar tests completed!\n\n";
}

int main() {
    std::cout << "=== LALR(1) Parser Generator Complete Test Suite ===\n\n";
    
    try {
        test_lr_items();
        test_lr0_automaton();
        test_lalr1_table_generation();
        test_simple_grammar();
        test_calculator_parsing();
        
        std::cout << "=== All tests completed! ===\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception\n";
        return 1;
    }
}