#include <iostream>
#include <cassert>
#include "token.h"
#include "lexer.h"
#include "symbol.h"
#include "grammar.h"

using namespace lalr1;

void test_token() {
    std::cout << "Testing Token class...\n";
    
    Token t1(TokenType::NUMBER, "123", 1, 5, 4);
    assert(t1.type == TokenType::NUMBER);
    assert(t1.value == "123");
    assert(t1.line == 1);
    assert(t1.column == 5);
    assert(t1.position == 4);
    assert(!t1.is_eof());
    assert(!t1.is_error());
    
    Token t2;
    assert(t2.is_eof());
    
    std::cout << "Token test: " << t1.to_string() << "\n";
    std::cout << "Token tests passed!\n\n";
}

void test_lexer() {
    std::cout << "Testing Lexer class...\n";
    
    auto lexer = create_calculator_lexer();
    lexer->set_input("2 + 3 * 4");
    
    std::vector<Token> tokens;
    Token token;
    do {
        token = lexer->next_token();
        tokens.push_back(token);
        std::cout << "  " << token.to_string() << "\n";
    } while (!token.is_eof());
    
    // Check token sequence
    assert(tokens.size() == 6);  // 2, +, 3, *, 4, EOF
    assert(tokens[0].type == TokenType::NUMBER);
    assert(tokens[0].value == "2");
    assert(tokens[1].type == TokenType::PLUS);
    assert(tokens[2].type == TokenType::NUMBER);
    assert(tokens[2].value == "3");
    assert(tokens[3].type == TokenType::MULTIPLY);
    assert(tokens[4].type == TokenType::NUMBER);
    assert(tokens[4].value == "4");
    assert(tokens[5].is_eof());
    
    std::cout << "Lexer tests passed!\n\n";
}

void test_symbol_table() {
    std::cout << "Testing SymbolTable class...\n";
    
    SymbolTable symbols;
    
    auto plus = symbols.get_terminal("+", TokenType::PLUS);
    auto num = symbols.get_terminal("num", TokenType::NUMBER);
    auto expr = symbols.get_nonterminal("E");
    auto term = symbols.get_nonterminal("T");
    
    assert(plus->is_terminal());
    assert(plus->name() == "+");
    assert(plus->token_type() == TokenType::PLUS);
    
    assert(expr->is_nonterminal());
    assert(expr->name() == "E");
    
    // Test symbol lookup
    auto found_plus = symbols.find_symbol("+");
    assert(found_plus == plus);
    
    // Test epsilon and end of input
    auto epsilon = symbols.get_epsilon();
    auto eof_sym = symbols.get_end_of_input();
    assert(epsilon->is_epsilon());
    assert(eof_sym->is_end_of_input());
    
    symbols.print_symbols();
    std::cout << "SymbolTable tests passed!\n\n";
}

void test_grammar() {
    std::cout << "Testing Grammar class...\n";
    
    Grammar grammar;
    
    // Create simple grammar: E -> E + T | T, T -> num
    auto E = grammar.symbol_table().get_nonterminal("E");
    auto T = grammar.symbol_table().get_nonterminal("T");
    auto plus = grammar.symbol_table().get_terminal("+", TokenType::PLUS);
    auto num = grammar.symbol_table().get_terminal("num", TokenType::NUMBER);
    
    // E -> E + T
    grammar.add_production(E, {E, plus, T});
    // E -> T
    grammar.add_production(E, {T});
    // T -> num
    grammar.add_production(T, {num});
    
    grammar.set_start_symbol(E);
    
    // Test grammar validation
    assert(grammar.is_valid());
    auto errors = grammar.get_validation_errors();
    assert(errors.empty());
    
    // Print grammar
    grammar.print_grammar();
    
    // Test FIRST and FOLLOW sets
    std::cout << "\nComputing FIRST sets...\n";
    grammar.print_first_sets();
    
    std::cout << "\nComputing FOLLOW sets...\n";
    grammar.print_follow_sets();
    
    // Test augmentation
    std::cout << "\nAugmenting grammar...\n";
    grammar.augment();
    assert(grammar.is_augmented());
    grammar.print_grammar();
    
    std::cout << "Grammar tests passed!\n\n";
}

int main() {
    std::cout << "=== LALR(1) Parser Generator Test Suite ===\n\n";
    
    try {
        test_token();
        test_lexer();
        test_symbol_table();
        test_grammar();
        
        std::cout << "=== All tests passed! ===\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception\n";
        return 1;
    }
}