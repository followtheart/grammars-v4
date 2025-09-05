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
#include "g4_parser.h"

using namespace lalr1;

void print_usage(const char* program_name) {
    std::cout << "ANTLR4 (.g4) to LALR(1) Parser Generator\n";
    std::cout << "Usage: " << program_name << " [options] [grammar_file.g4]\n";
    std::cout << "\nOptions:\n";
    std::cout << "  -h, --help          Show this help message\n";
    std::cout << "  --demo              Run demonstration with built-in calculator grammar\n";
    std::cout << "  --analyze           Analyze grammar for LALR(1) properties\n";
    std::cout << "  --convert-bnf FILE  Convert .g4 to BNF format and save to FILE\n";
    std::cout << "  --show-states       Show LALR(1) states\n";
    std::cout << "  --show-table        Show parsing table\n";
    std::cout << "  --show-sets         Show FIRST and FOLLOW sets\n";
    std::cout << "  --verbose           Enable verbose output\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << program_name << " grammar.g4\n";
    std::cout << "  " << program_name << " --analyze --verbose grammar.g4\n";
    std::cout << "  " << program_name << " --convert-bnf output.bnf grammar.g4\n";
}

void process_g4_file(const std::string& filename, bool verbose = false, 
                     bool show_states = false, bool show_table = false, bool show_sets = false) {
    std::cout << "=== Processing ANTLR4 Grammar File ===\n";
    std::cout << "File: " << filename << "\n\n";
    
    G4Parser parser;
    parser.set_verbose(verbose);
    
    auto grammar = parser.parse_file(filename);
    if (!grammar) {
        std::cout << "Failed to parse grammar file.\n";
        for (const auto& error : parser.get_errors()) {
            std::cout << "Error: " << error << "\n";
        }
        return;
    }
    
    std::cout << "Grammar parsed successfully!\n";
    grammar->print_grammar();
    std::cout << "\n";
    
    // 增强语法
    grammar->augment();
    
    if (show_sets) {
        std::cout << "FIRST Sets:\n";
        grammar->print_first_sets();
        std::cout << "\n";
        
        std::cout << "FOLLOW Sets:\n";
        grammar->print_follow_sets();
        std::cout << "\n";
    }
    
    try {
        // 生成LR(0)自动机
        std::cout << "Generating LR(0) Automaton...\n";
        LR0Automaton lr0_automaton(*grammar);
        
        if (show_states) {
            lr0_automaton.print_automaton();
            std::cout << "\n";
        }
        
        // 生成LALR(1)解析表
        std::cout << "Generating LALR(1) Parse Table...\n";
        LALR1Generator generator(*grammar);
        auto table = generator.generate_table();
        
        if (show_table) {
            table->print_table();
            std::cout << "\n";
        }
        
        if (table->has_conflicts()) {
            std::cout << "Grammar has conflicts:\n";
            table->print_conflicts();
            std::cout << "\n";
            std::cout << "This grammar is NOT LALR(1).\n";
        } else {
            std::cout << "No conflicts found - grammar is LALR(1)!\n";
        }
        
    } catch (const std::exception& e) {
        std::cout << "Error during parsing table generation: " << e.what() << "\n";
    }
    
    std::cout << "\n=== Processing Complete ===\n";
}

void analyze_g4_file(const std::string& filename) {
    std::cout << "=== Grammar Analysis ===\n";
    G4Utils::analyze_grammar(filename);
    std::cout << "=== Analysis Complete ===\n";
}

bool convert_g4_to_bnf(const std::string& input_file, const std::string& output_file) {
    std::cout << "Converting " << input_file << " to BNF format...\n";
    
    if (G4Utils::convert_to_bnf(input_file, output_file)) {
        std::cout << "Conversion successful: " << output_file << "\n";
        return true;
    } else {
        std::cout << "Conversion failed.\n";
        return false;
    }
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
    
    bool verbose = false;
    bool show_states = false;
    bool show_table = false;
    bool show_sets = false;
    bool analyze_mode = false;
    std::string convert_bnf_file;
    std::string grammar_file;
    
    // 解析命令行参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--demo") {
            run_demo();
            return 0;
        } else if (arg == "--verbose") {
            verbose = true;
        } else if (arg == "--show-states") {
            show_states = true;
        } else if (arg == "--show-table") {
            show_table = true;
        } else if (arg == "--show-sets") {
            show_sets = true;
        } else if (arg == "--analyze") {
            analyze_mode = true;
        } else if (arg == "--convert-bnf") {
            if (i + 1 < argc) {
                convert_bnf_file = argv[++i];
            } else {
                std::cerr << "Error: --convert-bnf requires output filename\n";
                return 1;
            }
        } else if (!arg.empty() && arg[0] != '-') {
            grammar_file = arg;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // 处理语法文件
    if (!grammar_file.empty()) {
        // 检查文件是否存在
        std::ifstream file(grammar_file);
        if (!file.good()) {
            std::cerr << "Error: Cannot open grammar file: " << grammar_file << "\n";
            return 1;
        }
        file.close();
        
        // 执行相应操作
        if (!convert_bnf_file.empty()) {
            return convert_g4_to_bnf(grammar_file, convert_bnf_file) ? 0 : 1;
        } else if (analyze_mode) {
            analyze_g4_file(grammar_file);
        } else {
            process_g4_file(grammar_file, verbose, show_states, show_table, show_sets);
        }
    } else if (convert_bnf_file.empty() && !analyze_mode) {
        std::cerr << "Error: No grammar file specified\n";
        print_usage(argv[0]);
        return 1;
    }
    
    return 0;
}