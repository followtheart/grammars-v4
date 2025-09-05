#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "token.h"
#include "lexer.h"
#include "symbol.h"
#include "grammar.h"
#include "lr_items.h"
#include "parse_table.h"

using namespace lalr1;

void print_usage(const char* program_name) {
    std::cout << "LALR(1) Parser Generator\n";
    std::cout << "Usage: " << program_name << " [options]\n";
    std::cout << "\nOptions:\n";
    std::cout << "  -h, --help       Show this help message\n";
    std::cout << "  --demo           Run demonstration with built-in calculator grammar\n";
    std::cout << "  --show-states    Show LALR(1) states\n";
    std::cout << "  --show-table     Show parsing table\n";
    std::cout << "  --show-sets      Show FIRST and FOLLOW sets\n";
}

void run_demo() {
    std::cout << "=== LALR(1) Parser Generator Demo ===\n\n";
    
    // Create simple calculator grammar
    Grammar grammar;
    auto E = grammar.symbol_table().get_nonterminal("E");
    auto T = grammar.symbol_table().get_nonterminal("T");
    auto plus = grammar.symbol_table().get_terminal("+", TokenType::PLUS);
    auto num = grammar.symbol_table().get_terminal("num", TokenType::NUMBER);
    
    // Grammar: E -> E + T | T, T -> num
    grammar.add_production(E, {E, plus, T});
    grammar.add_production(E, {T});
    grammar.add_production(T, {num});
    
    grammar.set_start_symbol(E);
    grammar.augment();
    
    std::cout << "Grammar:\n";
    grammar.print_grammar();
    std::cout << "\n";
    
    std::cout << "Symbol Table:\n";
    grammar.symbol_table().print_symbols();
    std::cout << "\n";
    
    // Generate LR(0) automaton
    std::cout << "LR(0) Automaton:\n";
    LR0Automaton lr0_automaton(grammar);
    lr0_automaton.print_automaton();
    std::cout << "\n";
    
    // Generate LALR(1) table
    LALR1Generator generator(grammar);
    
    std::cout << "FIRST Sets:\n";
    generator.print_first_sets();
    std::cout << "\n";
    
    std::cout << "FOLLOW Sets:\n";
    generator.print_follow_sets();
    std::cout << "\n";
    
    std::cout << "LALR(1) States:\n";
    generator.print_states();
    std::cout << "\n";
    
    auto table = generator.generate_table();
    
    std::cout << "LALR(1) Parsing Table:\n";
    table->print_table();
    std::cout << "\n";
    
    if (table->has_conflicts()) {
        std::cout << "Conflicts:\n";
        table->print_conflicts();
    } else {
        std::cout << "No conflicts found - grammar is LALR(1)!\n";
    }
    
    std::cout << "\n=== Demo Complete ===\n";
}

int main(int argc, char** argv) {
    if (argc == 1) {
        print_usage(argv[0]);
        return 1;
    }
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--demo") {
            run_demo();
            return 0;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }
    
    return 0;
}