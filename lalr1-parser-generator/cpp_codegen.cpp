#include "cpp_codegen.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <filesystem>

namespace lalr1 {

CppCodeGenerator::CppCodeGenerator(const Grammar& grammar, std::shared_ptr<ParseTable> table)
    : grammar_(grammar)
    , table_(table)
    , namespace_("generated")
    , class_name_("Parser")
    , generate_visitor_(false)
    , generate_listener_(false)
    , verbose_(false)
{
}

CppCodeGenerator::~CppCodeGenerator() = default;

bool CppCodeGenerator::generate_parser(const std::string& base_name, const std::string& output_dir) {
    try {
        // 确保输出目录存在
        std::filesystem::create_directories(output_dir);
        
        if (verbose_) {
            std::cout << "Generating C++ parser for grammar: " << base_name << std::endl;
        }
        
        // 生成各个文件
        std::string header_file = output_dir + "/" + base_name + "Parser.h";
        std::string source_file = output_dir + "/" + base_name + "Parser.cpp";
        std::string token_file = output_dir + "/" + base_name + "Tokens.h";
        std::string ast_file = output_dir + "/" + base_name + "AST.h";
        std::string lexer_header = output_dir + "/" + base_name + "Lexer.h";
        std::string lexer_source = output_dir + "/" + base_name + "Lexer.cpp";
        
        // 先生成lexer
        generate_lexer_header(lexer_header, base_name);
        generate_lexer_source(lexer_source, base_name);
        
        // 然后生成其他文件
        generate_token_header(token_file, base_name);
        generate_ast_nodes(ast_file, base_name);
        generate_header_file(header_file, base_name);
        generate_source_file(source_file, base_name);
        
        if (generate_visitor_) {
            std::string visitor_file = output_dir + "/" + base_name + "Visitor.h";
            generate_visitor_pattern(visitor_file, base_name);
        }
        
        if (generate_listener_) {
            std::string listener_file = output_dir + "/" + base_name + "Listener.h";
            generate_listener_pattern(listener_file, base_name);
        }
        
        // 生成构建文件
        BuildFileGenerator::generate_makefile(base_name, output_dir);
        BuildFileGenerator::generate_cmake(base_name, output_dir);
        
        // 生成测试用例
        std::string test_file = output_dir + "/" + base_name + "Test.cpp";
        generate_test_case(test_file, base_name);
        
        if (verbose_) {
            std::cout << "C++ parser generation completed successfully!" << std::endl;
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error generating C++ parser: " << e.what() << std::endl;
        return false;
    }
}

void CppCodeGenerator::generate_header_file(const std::string& filename, const std::string& base_name) {
    std::ofstream out(filename);
    if (!out) {
        throw std::runtime_error("Cannot create header file: " + filename);
    }
    
    write_file_header(out, "Generated LALR(1) Parser Header");
    
    // Include guards
    std::string guard_name = to_cpp_identifier(base_name) + "_PARSER_H";
    std::transform(guard_name.begin(), guard_name.end(), guard_name.begin(), ::toupper);
    
    out << "#pragma once\n\n";
    
    write_includes(out, true);
    out << "#include \"" << base_name << "Tokens.h\"\n";
    out << "#include \"" << base_name << "AST.h\"\n\n";
    
    write_namespace_begin(out);
    
    write_parser_class_declaration(out);
    
    write_namespace_end(out);
}

void CppCodeGenerator::generate_source_file(const std::string& filename, const std::string& base_name) {
    std::ofstream out(filename);
    if (!out) {
        throw std::runtime_error("Cannot create source file: " + filename);
    }
    
    write_file_header(out, "Generated LALR(1) Parser Implementation");
    
    out << "#include \"" << base_name << "Parser.h\"\n";
    write_includes(out, false);
    out << "\n";
    
    write_namespace_begin(out);
    
    write_parser_class_implementation(out);
    
    write_namespace_end(out);
}

void CppCodeGenerator::generate_token_header(const std::string& filename, const std::string& base_name) {
    std::ofstream out(filename);
    if (!out) {
        throw std::runtime_error("Cannot create token header file: " + filename);
    }
    
    write_file_header(out, "Generated Token Definitions");
    
    std::string guard_name = to_cpp_identifier(base_name) + "_TOKENS_H";
    std::transform(guard_name.begin(), guard_name.end(), guard_name.begin(), ::toupper);
    
    out << "#pragma once\n\n";
    out << "#include <string>\n\n";
    
    write_namespace_begin(out);
    
    write_token_definitions(out);
    
    write_namespace_end(out);
}

void CppCodeGenerator::generate_ast_nodes(const std::string& filename, const std::string& base_name) {
    std::ofstream out(filename);
    if (!out) {
        throw std::runtime_error("Cannot create AST header file: " + filename);
    }
    
    write_file_header(out, "Generated AST Node Definitions");
    
    std::string guard_name = to_cpp_identifier(base_name) + "_AST_H";
    std::transform(guard_name.begin(), guard_name.end(), guard_name.begin(), ::toupper);
    
    out << "#pragma once\n\n";
    out << "#include <memory>\n";
    out << "#include <vector>\n";
    out << "#include <string>\n\n";
    
    write_namespace_begin(out);
    
    write_ast_node_classes(out);
    
    write_namespace_end(out);
}

void CppCodeGenerator::generate_visitor_pattern(const std::string& filename, const std::string& base_name) {
    std::ofstream out(filename);
    if (!out) {
        throw std::runtime_error("Cannot create visitor header file: " + filename);
    }
    
    write_file_header(out, "Generated Visitor Pattern");
    
    std::string guard_name = to_cpp_identifier(base_name) + "_VISITOR_H";
    std::transform(guard_name.begin(), guard_name.end(), guard_name.begin(), ::toupper);
    
    out << "#pragma once\n\n";
    out << "#include \"" << base_name << "AST.h\"\n\n";
    
    write_namespace_begin(out);
    
    out << "/**\n";
    out << " * Visitor interface for AST traversal\n";
    out << " */\n";
    out << "template<typename T>\n";
    out << "class " << class_name_ << "Visitor {\n";
    out << "public:\n";
    out << "    virtual ~" << class_name_ << "Visitor() = default;\n\n";
    
    // Generate visit methods for each production
    for (const auto& production : grammar_.productions()) {
        std::string method_name = "visit" + to_class_name(production->lhs()->name());
        out << "    virtual T " << method_name << "(const " << get_ast_node_name(production->lhs()->name()) << "& node) = 0;\n";
    }
    
    out << "};\n\n";
    
    write_namespace_end(out);
}

void CppCodeGenerator::generate_listener_pattern(const std::string& filename, const std::string& base_name) {
    std::ofstream out(filename);
    if (!out) {
        throw std::runtime_error("Cannot create listener header file: " + filename);
    }
    
    write_file_header(out, "Generated Listener Pattern");
    
    std::string guard_name = to_cpp_identifier(base_name) + "_LISTENER_H";
    std::transform(guard_name.begin(), guard_name.end(), guard_name.begin(), ::toupper);
    
    out << "#pragma once\n\n";
    out << "#include \"" << base_name << "AST.h\"\n\n";
    
    write_namespace_begin(out);
    
    out << "/**\n";
    out << " * Listener interface for AST traversal\n";
    out << " */\n";
    out << "class " << class_name_ << "Listener {\n";
    out << "public:\n";
    out << "    virtual ~" << class_name_ << "Listener() = default;\n\n";
    
    // Generate enter/exit methods for each production
    for (const auto& production : grammar_.productions()) {
        std::string class_name = to_class_name(production->lhs()->name());
        out << "    virtual void enter" << class_name << "(const " << get_ast_node_name(production->lhs()->name()) << "& node) {}\n";
        out << "    virtual void exit" << class_name << "(const " << get_ast_node_name(production->lhs()->name()) << "& node) {}\n\n";
    }
    
    out << "};\n\n";
    
    write_namespace_end(out);
}

void CppCodeGenerator::write_file_header(std::ofstream& out, const std::string& description) {
    out << "/*\n";
    out << " * " << description << "\n";
    out << " * Generated by LALR(1) Parser Generator\n";
    out << " * \n";
    out << " * DO NOT EDIT THIS FILE MANUALLY\n";
    out << " * This file was automatically generated from a .g4 grammar file.\n";
    out << " */\n\n";
}

void CppCodeGenerator::write_includes(std::ofstream& out, bool is_header) {
    if (is_header) {
        out << "#include <memory>\n";
        out << "#include <vector>\n";
        out << "#include <string>\n";
        out << "#include <stack>\n";
        out << "#include <iostream>\n";
        out << "#include <stdexcept>\n";
    } else {
        out << "#include <iostream>\n";
        out << "#include <sstream>\n";
        out << "#include <algorithm>\n";
    }
    out << "\n";
}

void CppCodeGenerator::write_namespace_begin(std::ofstream& out) {
    if (!namespace_.empty()) {
        out << "namespace " << namespace_ << " {\n\n";
    }
}

void CppCodeGenerator::write_namespace_end(std::ofstream& out) {
    if (!namespace_.empty()) {
        out << "} // namespace " << namespace_ << "\n";
    }
}

void CppCodeGenerator::write_token_definitions(std::ofstream& out) {
    out << "/**\n";
    out << " * Token type enumeration\n";
    out << " */\n";
    out << "enum class TokenType {\n";
    
    // EOF token
    out << "    EOF_TOKEN = 0,\n";
    
    // Terminal symbols
    int token_id = 1;
    for (const auto& symbol : grammar_.symbol_table().get_all_symbols()) {
        if (symbol->is_terminal() && symbol->name() != "$") {
            out << "    " << to_token_name(symbol->name()) << " = " << token_id++ << ",\n";
        }
    }
    
    out << "};\n\n";
    
    out << "/**\n";
    out << " * Token structure\n";
    out << " */\n";
    out << "struct Token {\n";
    out << "    TokenType type;\n";
    out << "    std::string text;\n";
    out << "    size_t line;\n";
    out << "    size_t column;\n";
    out << "\n";
    out << "    Token(TokenType t = TokenType::EOF_TOKEN, const std::string& txt = \"\", size_t ln = 0, size_t col = 0)\n";
    out << "        : type(t), text(txt), line(ln), column(col) {}\n";
    out << "};\n\n";
    
    out << "/**\n";
    out << " * Convert token type to string\n";
    out << " */\n";
    out << "inline std::string token_type_to_string(TokenType type) {\n";
    out << "    switch (type) {\n";
    out << "        case TokenType::EOF_TOKEN: return \"EOF\";\n";
    
    for (const auto& symbol : grammar_.symbol_table().get_all_symbols()) {
        if (symbol->is_terminal() && symbol->name() != "$") {
            out << "        case TokenType::" << to_token_name(symbol->name()) << ": return \"" << symbol->name() << "\";\n";
        }
    }
    
    out << "        default: return \"UNKNOWN\";\n";
    out << "    }\n";
    out << "}\n\n";
}

void CppCodeGenerator::write_ast_node_classes(std::ofstream& out) {
    out << "/**\n";
    out << " * Base AST Node class\n";
    out << " */\n";
    out << "class ASTNode {\n";
    out << "public:\n";
    out << "    virtual ~ASTNode() = default;\n";
    out << "    virtual std::string to_string() const = 0;\n";
    out << "};\n\n";
    
    out << "using ASTNodePtr = std::shared_ptr<ASTNode>;\n\n";
    
    // Generate AST node classes for each production
    std::set<std::string> generated_classes;
    for (const auto& production : grammar_.productions()) {
        std::string rule_name = production->lhs()->name();
        if (generated_classes.find(rule_name) != generated_classes.end()) {
            continue; // Already generated
        }
        generated_classes.insert(rule_name);
        
        std::string class_name = get_ast_node_name(rule_name);
        
        out << "/**\n";
        out << " * AST Node for rule: " << rule_name << "\n";
        out << " */\n";
        out << "class " << class_name << " : public ASTNode {\n";
        out << "public:\n";
        out << "    std::vector<ASTNodePtr> children;\n";
        out << "    std::string rule_name;\n\n";
        
        out << "    " << class_name << "() : rule_name(\"" << rule_name << "\") {}\n\n";
        
        out << "    void add_child(ASTNodePtr child) {\n";
        out << "        if (child) children.push_back(child);\n";
        out << "    }\n\n";
        
        out << "    std::string to_string() const override {\n";
        out << "        std::string result = rule_name;\n";
        out << "        if (!children.empty()) {\n";
        out << "            result += \"(\";\n";
        out << "            for (size_t i = 0; i < children.size(); ++i) {\n";
        out << "                if (i > 0) result += \", \";\n";
        out << "                result += children[i]->to_string();\n";
        out << "            }\n";
        out << "            result += \")\";\n";
        out << "        }\n";
        out << "        return result;\n";
        out << "    }\n";
        out << "};\n\n";
    }
    
    // Terminal node for tokens
    out << "/**\n";
    out << " * Terminal AST Node for tokens\n";
    out << " */\n";
    out << "class TerminalNode : public ASTNode {\n";
    out << "public:\n";
    out << "    Token token;\n\n";
    out << "    TerminalNode(const Token& t) : token(t) {}\n\n";
    out << "    std::string to_string() const override {\n";
    out << "        return token.text.empty() ? token_type_to_string(token.type) : token.text;\n";
    out << "    }\n";
    out << "};\n\n";
}

void CppCodeGenerator::write_parser_class_declaration(std::ofstream& out) {
    out << "/**\n";
    out << " * LALR(1) Parser class\n";
    out << " */\n";
    out << "class " << class_name_ << " {\n";
    out << "public:\n";
    out << "    " << class_name_ << "();\n";
    out << "    ~" << class_name_ << "();\n\n";
    
    out << "    /**\n";
    out << "     * Parse input tokens and return AST\n";
    out << "     */\n";
    out << "    ASTNodePtr parse(const std::vector<Token>& tokens);\n\n";
    
    out << "    /**\n";
    out << "     * Enable/disable verbose output\n";
    out << "     */\n";
    out << "    void set_verbose(bool verbose) { verbose_ = verbose; }\n\n";
    
    out << "    /**\n";
    out << "     * Get error messages\n";
    out << "     */\n";
    out << "    const std::vector<std::string>& get_errors() const { return errors_; }\n\n";
    
    out << "private:\n";
    out << "    struct ParseState {\n";
    out << "        int state;\n";
    out << "        ASTNodePtr node;\n";
    out << "        ParseState(int s, ASTNodePtr n = nullptr) : state(s), node(n) {}\n";
    out << "    };\n\n";
    
    out << "    std::stack<ParseState> state_stack_;\n";
    out << "    std::vector<Token> tokens_;\n";
    out << "    size_t current_token_;\n";
    out << "    bool verbose_;\n";
    out << "    std::vector<std::string> errors_;\n\n";
    
    out << "    // Parsing tables\n";
    auto terminals = grammar_.symbol_table().get_terminals();
    int num_terminals = 1; // EOF_TOKEN
    for (const auto& symbol : terminals) {
        if (symbol->name() != "$") {
            num_terminals++;
        }
    }
    out << "    static const int ACTION_TABLE[][" << num_terminals << "];\n";
    out << "    static const int GOTO_TABLE[][" << grammar_.symbol_table().get_nonterminals().size() << "];\n";
    out << "    static const int PRODUCTION_LHS[];\n";
    out << "    static const int PRODUCTION_LENGTH[];\n\n";
    
    out << "    // Helper methods\n";
    out << "    void shift(int state);\n";
    out << "    void reduce(int production_id);\n";
    out << "    ASTNodePtr create_production_node(int production_id, const std::vector<ASTNodePtr>& children);\n";
    out << "    void error(const std::string& message);\n";
    out << "    TokenType get_current_token_type() const;\n";
    out << "    int get_action(int state, TokenType token) const;\n";
    out << "    int get_goto(int state, int non_terminal) const;\n";
    out << "    int token_to_index(TokenType token) const;\n";
    out << "};\n\n";
}

void CppCodeGenerator::write_parser_class_implementation(std::ofstream& out) {
    out << class_name_ << "::" << class_name_ << "()\n";
    out << "    : current_token_(0)\n";
    out << "    , verbose_(false)\n";
    out << "{\n";
    out << "    state_stack_.push(ParseState(0));\n";
    out << "}\n\n";
    
    out << class_name_ << "::~" << class_name_ << "() = default;\n\n";
    
    write_parsing_methods(out);
    write_action_table(out);
    write_goto_table(out);
    write_production_rules(out);
    write_error_handling(out);
}

void CppCodeGenerator::write_parsing_methods(std::ofstream& out) {
    out << "ASTNodePtr " << class_name_ << "::parse(const std::vector<Token>& tokens) {\n";
    out << "    tokens_ = tokens;\n";
    out << "    current_token_ = 0;\n";
    out << "    errors_.clear();\n\n";
    
    out << "    // Clear state stack and initialize with state 0\n";
    out << "    while (!state_stack_.empty()) state_stack_.pop();\n";
    out << "    state_stack_.push(ParseState(0));\n\n";
    
    out << "    while (true) {\n";
    out << "        int current_state = state_stack_.top().state;\n";
    out << "        TokenType current_token = get_current_token_type();\n";
    out << "        int action = get_action(current_state, current_token);\n\n";
    
    out << "        if (verbose_) {\n";
    out << "            std::cout << \"State: \" << current_state \n";
    out << "                      << \", Token: \" << token_type_to_string(current_token)\n";
    out << "                      << \", Action: \" << action << std::endl;\n";
    out << "        }\n\n";
    
    out << "        if (action > 0) {\n";
    out << "            // Shift action\n";
    out << "            shift(action);\n";
    out << "        } else if (action < -1) {\n";
    out << "            // Reduce action\n";
    out << "            int production_id = -(action + 1);\n";
    out << "            reduce(production_id);\n";
    out << "        } else if (action == 0) {\n";
    out << "            // Accept\n";
    out << "            if (state_stack_.size() >= 2) {\n";
    out << "                return state_stack_.top().node;\n";
    out << "            } else {\n";
    out << "                error(\"Parse completed but no result available\");\n";
    out << "                return nullptr;\n";
    out << "            }\n";
    out << "        } else {\n";
    out << "            // Error\n";
    out << "            error(\"Unexpected token: \" + token_type_to_string(current_token));\n";
    out << "            return nullptr;\n";
    out << "        }\n";
    out << "    }\n";
    out << "}\n\n";
    
    out << "void " << class_name_ << "::shift(int state) {\n";
    out << "    if (current_token_ < tokens_.size()) {\n";
    out << "        auto terminal_node = std::make_shared<TerminalNode>(tokens_[current_token_]);\n";
    out << "        state_stack_.push(ParseState(state, terminal_node));\n";
    out << "        current_token_++;\n";
    out << "    }\n";
    out << "}\n\n";
    
    out << "void " << class_name_ << "::reduce(int production_id) {\n";
    out << "    int lhs = PRODUCTION_LHS[production_id];\n";
    out << "    int length = PRODUCTION_LENGTH[production_id];\n\n";
    
    out << "    std::vector<ASTNodePtr> children;\n";
    out << "    for (int i = 0; i < length; ++i) {\n";
    out << "        if (!state_stack_.empty()) {\n";
    out << "            children.insert(children.begin(), state_stack_.top().node);\n";
    out << "            state_stack_.pop();\n";
    out << "        }\n";
    out << "    }\n\n";
    
    out << "    ASTNodePtr node = create_production_node(production_id, children);\n";
    out << "    int current_state = state_stack_.empty() ? 0 : state_stack_.top().state;\n";
    out << "    int goto_state = get_goto(current_state, lhs);\n\n";
    
    out << "    if (goto_state >= 0) {\n";
    out << "        state_stack_.push(ParseState(goto_state, node));\n";
    out << "    } else {\n";
    out << "        error(\"Invalid goto state\");\n";
    out << "    }\n";
    out << "}\n\n";
    
    out << "ASTNodePtr " << class_name_ << "::create_production_node(int production_id, const std::vector<ASTNodePtr>& children) {\n";
    
    // Generate cases for each production
    out << "    switch (production_id) {\n";
    int prod_id = 1;
    for (const auto& production : grammar_.productions()) {
        if (production->lhs()->name() == grammar_.start_symbol()->name() + "'") {
            // Skip augmented start production
            prod_id++;
            continue;
        }
        
        std::string node_name = get_ast_node_name(production->lhs()->name());
        out << "        case " << prod_id << ": {\n";
        out << "            auto node = std::make_shared<" << node_name << ">();\n";
        out << "            for (const auto& child : children) {\n";
        out << "                node->add_child(child);\n";
        out << "            }\n";
        out << "            return node;\n";
        out << "        }\n";
        prod_id++;
    }
    
    out << "        default:\n";
    out << "            error(\"Unknown production ID: \" + std::to_string(production_id));\n";
    out << "            return nullptr;\n";
    out << "    }\n";
    out << "}\n\n";
    
    out << "TokenType " << class_name_ << "::get_current_token_type() const {\n";
    out << "    if (current_token_ < tokens_.size()) {\n";
    out << "        return tokens_[current_token_].type;\n";
    out << "    }\n";
    out << "    return TokenType::EOF_TOKEN;\n";
    out << "}\n\n";
    
    out << "int " << class_name_ << "::get_action(int state, TokenType token) const {\n";
    out << "    if (state >= 0 && state < " << table_->num_states() << ") {\n";
    out << "        int token_index = token_to_index(token);\n";
    out << "        if (token_index >= 0) {\n";
    out << "            return ACTION_TABLE[state][token_index];\n";
    out << "        }\n";
    out << "    }\n";
    out << "    return -1; // Error\n";
    out << "}\n\n";
    
    out << "int " << class_name_ << "::get_goto(int state, int non_terminal) const {\n";
    out << "    if (state >= 0 && state < " << table_->num_states() << ") {\n";
    out << "        return GOTO_TABLE[state][non_terminal];\n";
    out << "    }\n";
    out << "    return -1; // Error\n";
    out << "}\n\n";
    
    out << "int " << class_name_ << "::token_to_index(TokenType token) const {\n";
    out << "    switch (token) {\n";
    
    // Generate mapping for terminal symbols
    auto terminals = grammar_.symbol_table().get_terminals();
    std::vector<SymbolPtr> terminal_vec;
    
    // Add other terminals first (exclude $ since it will be mapped to EOF_TOKEN)
    for (const auto& symbol : terminals) {
        if (symbol->name() != "$") {
            terminal_vec.push_back(symbol);
        }
    }
    
    int index = 0;
    for (const auto& symbol : terminal_vec) {
        out << "        case TokenType::" << to_token_name(symbol->name()) << ": return " << index << ";\n";
        index++;
    }
    
    // EOF_TOKEN corresponds to $ in the grammar and comes last in our table
    out << "        case TokenType::EOF_TOKEN: return " << index << ";\n";
    
    out << "        default: return -1; // Unknown token\n";
    out << "    }\n";
    out << "}\n\n";
}

void CppCodeGenerator::write_action_table(std::ofstream& out) {
    out << "// LALR(1) Action Table\n";
    auto terminals = grammar_.symbol_table().get_terminals();
    std::vector<SymbolPtr> terminal_vec;
    
    // Add EOF_TOKEN first (this corresponds to $ in the grammar)
    auto eof_symbol = grammar_.symbol_table().get_end_of_input();
    
    // Add other terminals (exclude $ since it's already added as EOF)
    for (const auto& symbol : terminals) {
        if (symbol->name() != "$") {
            terminal_vec.push_back(symbol);
        }
    }
    
    // Add EOF at the end to match the original table format
    terminal_vec.push_back(eof_symbol);
    
    out << "const int " << class_name_ << "::ACTION_TABLE[][" << terminal_vec.size() << "] = {\n";
    
    if (verbose_) {
        out << "    // Terminal order: ";
        for (size_t i = 0; i < terminal_vec.size(); ++i) {
            if (i > 0) out << ", ";
            out << terminal_vec[i]->name();
        }
        out << "\n";
    }
    
    for (int state = 0; state < table_->num_states(); ++state) {
        out << "    { ";
        
        for (size_t i = 0; i < terminal_vec.size(); ++i) {
            if (i > 0) out << ", ";
            
            auto action = table_->get_action(state, terminal_vec[i]);
            if (action.type == ActionType::SHIFT) {
                out << std::setw(3) << action.value;
            } else if (action.type == ActionType::REDUCE) {
                out << std::setw(3) << -(action.value + 1); // Start from -2 to avoid conflict with error (-1)
            } else if (action.type == ActionType::ACCEPT) {
                out << std::setw(3) << 0;
            } else {
                out << std::setw(3) << -1; // Error
            }
        }
        
        out << " }";
        if (state < table_->num_states() - 1) out << ",";
        out << "\n";
    }
    
    out << "};\n\n";
}

void CppCodeGenerator::write_goto_table(std::ofstream& out) {
    out << "// LALR(1) Goto Table\n";
    auto non_terminals = grammar_.symbol_table().get_nonterminals();
    std::vector<SymbolPtr> nt_vec(non_terminals.begin(), non_terminals.end());
    
    // Sort to ensure consistent ordering: augmented start symbol first, then others alphabetically
    std::sort(nt_vec.begin(), nt_vec.end(), [](const SymbolPtr& a, const SymbolPtr& b) {
        // Put augmented start symbol first (contains ')
        bool a_augmented = a->name().find('\'') != std::string::npos;
        bool b_augmented = b->name().find('\'') != std::string::npos;
        
        if (a_augmented && !b_augmented) {
            return true;
        }
        if (!a_augmented && b_augmented) {
            return false;
        }
        return a->name() < b->name();
    });
    
    out << "const int " << class_name_ << "::GOTO_TABLE[][" << nt_vec.size() << "] = {\n";
    
    if (verbose_) {
        out << "    // Non-terminal order: ";
        for (size_t i = 0; i < nt_vec.size(); ++i) {
            if (i > 0) out << ", ";
            out << nt_vec[i]->name() << "(" << i << ")";
        }
        out << "\n";
        out << "    // Using same state ordering as display system (by state ID)\n";
    }
    
    // CRITICAL FIX: Generate table in same order as display system - by state ID, not vector order
    for (int state_id = 0; state_id < table_->num_states(); ++state_id) {
        out << "    { ";
        
        for (size_t i = 0; i < nt_vec.size(); ++i) {
            if (i > 0) out << ", ";
            
            int goto_state = table_->get_goto(state_id, nt_vec[i]);
            out << std::setw(3) << goto_state;
            
            if (verbose_ && goto_state >= 0) {
                std::cout << "Code-gen GOTO[" << state_id << "][" << nt_vec[i]->name() << "] = " << goto_state << std::endl;
            }
        }
        
        out << " }";
        if (state_id < table_->num_states() - 1) out << ",";
        if (verbose_) {
            out << " // State " << state_id;
        }
        out << "\n";
    }
    
    out << "};\n\n";
}

void CppCodeGenerator::write_production_rules(std::ofstream& out) {
    out << "// Production left-hand sides\n";
    out << "const int " << class_name_ << "::PRODUCTION_LHS[] = {\n";
    // out << "    0, // Dummy for production 0\n";
    
    auto non_terminals = grammar_.symbol_table().get_nonterminals();
    std::vector<SymbolPtr> nt_vec(non_terminals.begin(), non_terminals.end());
    
    // Sort to ensure consistent ordering: augmented start symbol first, then others alphabetically
    std::sort(nt_vec.begin(), nt_vec.end(), [](const SymbolPtr& a, const SymbolPtr& b) {
        // Put augmented start symbol first (contains ')
        bool a_augmented = a->name().find('\'') != std::string::npos;
        bool b_augmented = b->name().find('\'') != std::string::npos;
        
        if (a_augmented && !b_augmented) {
            return true;
        }
        if (!a_augmented && b_augmented) {
            return false;
        }
        return a->name() < b->name();
    });
    
    if (verbose_) {
        out << "    // Non-terminal order: ";
        for (size_t i = 0; i < nt_vec.size(); ++i) {
            if (i > 0) out << ", ";
            out << nt_vec[i]->name() << "(" << i << ")";
        }
        out << "\n";
    }
    std::sort(nt_vec.begin(), nt_vec.end(), [](const SymbolPtr& a, const SymbolPtr& b) {
        // Put augmented start symbol first (contains ')
        bool a_augmented = a->name().find('\'') != std::string::npos;
        bool b_augmented = b->name().find('\'') != std::string::npos;
        
        if (a_augmented && !b_augmented) {
            return true;
        }
        if (!a_augmented && b_augmented) {
            return false;
        }
        return a->name() < b->name();
    });
    
    for (const auto& production : grammar_.productions()) {
        auto it = std::find(nt_vec.begin(), nt_vec.end(), production->lhs());
        int lhs_index = std::distance(nt_vec.begin(), it);
        out << "    " << lhs_index << ", // " << production->to_string() << "\n";
    }
    
    out << "};\n\n";
    
    out << "// Production lengths\n";
    out << "const int " << class_name_ << "::PRODUCTION_LENGTH[] = {\n";
    // out << "    0, // Dummy for production 0\n";
    
    for (const auto& production : grammar_.productions()) {
        out << "    " << production->length() << ", // " << production->to_string() << "\n";
    }
    
    out << "};\n\n";
}

void CppCodeGenerator::write_error_handling(std::ofstream& out) {
    out << "void " << class_name_ << "::error(const std::string& message) {\n";
    out << "    std::string error_msg = \"Parse error\";\n";
    out << "    if (current_token_ < tokens_.size()) {\n";
    out << "        error_msg += \" at line \" + std::to_string(tokens_[current_token_].line)\n";
    out << "                  + \", column \" + std::to_string(tokens_[current_token_].column);\n";
    out << "    }\n";
    out << "    error_msg += \": \" + message;\n";
    out << "    errors_.push_back(error_msg);\n\n";
    
    out << "    if (verbose_) {\n";
    out << "        std::cerr << error_msg << std::endl;\n";
    out << "    }\n";
    out << "}\n\n";
}

// 工具函数实现
std::string CppCodeGenerator::to_cpp_identifier(const std::string& name) {
    // Handle special literal tokens
    if (name == "+") return "PLUS";
    if (name == "-") return "MINUS";
    if (name == "*") return "MULTIPLY";
    if (name == "/") return "DIVIDE";
    if (name == "(") return "LPAREN";
    if (name == ")") return "RPAREN";
    if (name == "{") return "LBRACE";
    if (name == "}") return "RBRACE";
    if (name == "[") return "LBRACKET";
    if (name == "]") return "RBRACKET";
    if (name == ";") return "SEMICOLON";
    if (name == ",") return "COMMA";
    if (name == ".") return "DOT";
    if (name == ":") return "COLON";
    if (name == "=") return "EQUALS";
    if (name == "<") return "LT";
    if (name == ">") return "GT";
    if (name == "<=") return "LE";
    if (name == ">=") return "GE";
    if (name == "==") return "EQ";
    if (name == "!=") return "NE";
    if (name == "&&") return "AND";
    if (name == "||") return "OR";
    if (name == "!") return "NOT";
    
    // For other names, convert to valid C++ identifier
    std::string result;
    for (char c : name) {
        if (std::isalnum(c) || c == '_') {
            result += c;
        } else {
            result += '_';
        }
    }
    if (!result.empty() && std::isdigit(result[0])) {
        result = "_" + result;
    }
    return result;
}

std::string CppCodeGenerator::to_token_name(const std::string& name) {
    std::string result = to_cpp_identifier(name);
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    
    // Handle C++ reserved words and common conflicts
    if (result == "NULL") {
        result = "NULL_TOKEN";
    } else if (result == "TRUE") {
        result = "TRUE_TOKEN";
    } else if (result == "FALSE") {
        result = "FALSE_TOKEN";
    }
    
    return result;
}

std::string CppCodeGenerator::to_class_name(const std::string& name) {
    std::string result = to_cpp_identifier(name);
    if (!result.empty()) {
        result[0] = std::toupper(result[0]);
    }
    return result;
}

std::string CppCodeGenerator::get_production_method_name(const ProductionPtr& production) {
    return "parse_" + to_cpp_identifier(production->lhs()->name());
}

std::string CppCodeGenerator::get_ast_node_name(const std::string& rule_name) {
    return to_class_name(rule_name) + "Node";
}

void CppCodeGenerator::write_indent(std::ofstream& out, int level) {
    for (int i = 0; i < level; ++i) {
        out << "    ";
    }
}

// BuildFileGenerator 实现
bool BuildFileGenerator::generate_makefile(const std::string& base_name, const std::string& output_dir) {
    std::string filename = output_dir + "/Makefile";
    std::ofstream out(filename);
    if (!out) return false;
    
    out << "# Generated Makefile for " << base_name << " Parser\n\n";
    out << "CXX = g++\n";
    out << "CXXFLAGS = -std=c++17 -Wall -Wextra -O2\n";
    out << "TARGET = " << base_name << "Parser\n";
    out << "SOURCES = " << base_name << "Parser.cpp\n";
    out << "HEADERS = " << base_name << "Parser.h " << base_name << "Tokens.h " << base_name << "AST.h\n\n";
    
    out << "all: $(TARGET)\n\n";
    out << "$(TARGET): $(SOURCES) $(HEADERS)\n";
    out << "\t$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)\n\n";
    
    out << "clean:\n";
    out << "\trm -f $(TARGET)\n\n";
    
    out << ".PHONY: all clean\n";
    
    return true;
}

bool BuildFileGenerator::generate_cmake(const std::string& base_name, const std::string& output_dir) {
    std::string filename = output_dir + "/CMakeLists.txt";
    std::ofstream out(filename);
    if (!out) return false;
    
    out << "# Generated CMakeLists.txt for " << base_name << " Parser\n\n";
    out << "cmake_minimum_required(VERSION 3.10)\n";
    out << "project(" << base_name << "Parser)\n\n";
    
    out << "set(CMAKE_CXX_STANDARD 17)\n";
    out << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n";
    
    // out << "add_executable(" << base_name << "Parser\n";
    // out << "    " << base_name << "Parser.cpp\n";
    // out << ")\n\n";
    
    out << "# Test executable\n";
    out << "add_executable(" << base_name << "Test\n";
    out << "    " << base_name << "Test.cpp\n";
    out << "    " << base_name << "Parser.cpp\n";
    out << "    " << base_name << "Lexer.cpp\n";
    out << ")\n\n";
    
    // out << "target_include_directories(" << base_name << "Parser PRIVATE .)\n";
    out << "target_include_directories(" << base_name << "Test PRIVATE .)\n\n";
    
    out << "if(MSVC)\n";
    // out << "    target_compile_options(" << base_name << "Parser PRIVATE /W4)\n";
    out << "    target_compile_options(" << base_name << "Test PRIVATE /W4)\n";
    out << "else()\n";
    // out << "    target_compile_options(" << base_name << "Parser PRIVATE -Wall -Wextra -Wpedantic)\n";
    out << "    target_compile_options(" << base_name << "Test PRIVATE -Wall -Wextra -Wpedantic)\n";
    out << "endif()\n\n";
    
    out << "# Enable testing\n";
    out << "enable_testing()\n";
    out << "add_test(NAME " << base_name << "_test COMMAND " << base_name << "Test)\n";
    
    return true;
}

void CppCodeGenerator::generate_lexer_header(const std::string& filename, const std::string& base_name) {
    std::ofstream out(filename);
    if (!out) {
        throw std::runtime_error("Cannot create lexer header file: " + filename);
    }
    
    write_file_header(out, "Generated Lexer Header");
    
    std::string guard_name = to_cpp_identifier(base_name) + "_LEXER_H";
    std::transform(guard_name.begin(), guard_name.end(), guard_name.begin(), ::toupper);
    
    out << "#pragma once\n\n";
    out << "#include <string>\n";
    out << "#include <vector>\n";
    out << "#include <regex>\n";
    out << "#include \"" << base_name << "Tokens.h\"\n\n";
    
    write_namespace_begin(out);
    
    out << "/**\n";
    out << " * Lexical analyzer for " << base_name << " language\n";
    out << " */\n";
    out << "class " << base_name << "Lexer {\n";
    out << "public:\n";
    out << "    " << base_name << "Lexer();\n";
    out << "    ~" << base_name << "Lexer();\n\n";
    
    out << "    /**\n";
    out << "     * Tokenize input string\n";
    out << "     */\n";
    out << "    std::vector<Token> tokenize(const std::string& input);\n\n";
    
    out << "    /**\n";
    out << "     * Set verbose mode for debugging\n";
    out << "     */\n";
    out << "    void set_verbose(bool verbose) { verbose_ = verbose; }\n\n";
    
    out << "    /**\n";
    out << "     * Get error messages\n";
    out << "     */\n";
    out << "    const std::vector<std::string>& get_errors() const { return errors_; }\n\n";
    
    out << "private:\n";
    out << "    struct TokenRule {\n";
    out << "        std::regex pattern;\n";
    out << "        TokenType type;\n";
    out << "        std::string name;\n";
    out << "        bool skip;\n";
    out << "        \n";
    out << "        TokenRule(const std::string& pat, TokenType t, const std::string& n, bool s = false)\n";
    out << "            : pattern(pat), type(t), name(n), skip(s) {}\n";
    out << "    };\n\n";
    
    out << "    std::vector<TokenRule> rules_;\n";
    out << "    bool verbose_;\n";
    out << "    std::vector<std::string> errors_;\n\n";
    
    out << "    void initialize_rules();\n";
    out << "    void error(const std::string& message, size_t line, size_t column);\n";
    out << "};\n\n";
    
    write_namespace_end(out);
    
    if (verbose_) {
        std::cout << "Generated lexer header: " << filename << std::endl;
    }
}

void CppCodeGenerator::generate_lexer_source(const std::string& filename, const std::string& base_name) {
    std::ofstream out(filename);
    if (!out) {
        throw std::runtime_error("Cannot create lexer source file: " + filename);
    }
    
    write_file_header(out, "Generated Lexer Implementation");
    
    out << "#include \"" << base_name << "Lexer.h\"\n";
    out << "#include <iostream>\n";
    out << "#include <sstream>\n";
    out << "#include <cctype>\n\n";
    
    write_namespace_begin(out);
    
    // Constructor
    out << base_name << "Lexer::" << base_name << "Lexer() : verbose_(false) {\n";
    out << "    initialize_rules();\n";
    out << "}\n\n";
    
    out << base_name << "Lexer::~" << base_name << "Lexer() = default;\n\n";
    
    // Initialize lexer rules based on grammar terminals
    out << "void " << base_name << "Lexer::initialize_rules() {\n";
    out << "    // Token rules based on grammar terminals\n";
    
    // Add whitespace rule first (skip)
    out << "    rules_.emplace_back(R\"(\\s+)\", TokenType::EOF_TOKEN, \"WHITESPACE\", true);\n";
    
    // Add rules for each terminal symbol
    auto terminals = grammar_.symbol_table().get_terminals();
    for (const auto& symbol : terminals) {
        if (symbol->name() != "$") {
            std::string token_name = to_token_name(symbol->name());
            std::string pattern;
            
            // Generate regex patterns based on token names
            if (token_name == "NUM" || token_name == "NUMBER" || token_name == "INT") {
                pattern = R"(\d+)";
            } else if (token_name == "ID" || token_name == "IDENTIFIER") {
                pattern = R"([a-zA-Z_][a-zA-Z0-9_]*)";
            } else if (token_name == "STRING") {
                pattern = R"("([^"\\]|\\.)*")";
            } else {
                // For literal tokens, escape special regex characters
                std::string escaped = symbol->name();
                std::string special_chars = ".*+?^${}()|[]\\";
                for (char c : special_chars) {
                    size_t pos = 0;
                    while ((pos = escaped.find(c, pos)) != std::string::npos) {
                        escaped.replace(pos, 1, "\\" + std::string(1, c));
                        pos += 2;
                    }
                }
                pattern = escaped;
            }
            
            // Use raw string literal for complex patterns, regular string for simple literals
            if(token_name == "NUM" || token_name == "NUMBER" || token_name == "INT" || token_name == "ID" || token_name == "IDENTIFIER"
                || token_name == "STRING") {
                out << "    rules_.emplace_back(R\"(" << pattern << ")\", TokenType::" 
                    << token_name << ", \"" << symbol->name() << "\");\n";
            } else {
                out << "    rules_.emplace_back(\"" << pattern << "\", TokenType::"
                    << token_name << ", \"" << symbol->name() << "\");\n";
            }
        }
    }
    
    out << "}\n\n";
    
    // Tokenize method
    out << "std::vector<Token> " << base_name << "Lexer::tokenize(const std::string& input) {\n";
    out << "    std::vector<Token> tokens;\n";
    out << "    errors_.clear();\n";
    out << "    \n";
    out << "    size_t pos = 0;\n";
    out << "    size_t line = 1;\n";
    out << "    size_t column = 1;\n";
    out << "    \n";
    out << "    while (pos < input.size()) {\n";
    out << "        bool matched = false;\n";
    out << "        \n";
    out << "        for (const auto& rule : rules_) {\n";
    out << "            std::smatch match;\n";
    out << "            std::string remaining = input.substr(pos);\n";
    out << "            \n";
    out << "            if (std::regex_search(remaining, match, rule.pattern) && match.position() == 0) {\n";
    out << "                std::string token_text = match.str();\n";
    out << "                \n";
    out << "                if (verbose_) {\n";
    out << "                    std::cout << \"Matched: '\" << token_text << \"' as \" << rule.name << std::endl;\n";
    out << "                }\n";
    out << "                \n";
    out << "                if (!rule.skip) {\n";
    out << "                    tokens.emplace_back(rule.type, token_text, line, column);\n";
    out << "                }\n";
    out << "                \n";
    out << "                // Update position and line/column counters\n";
    out << "                for (char c : token_text) {\n";
    out << "                    if (c == '\\n') {\n";
    out << "                        line++;\n";
    out << "                        column = 1;\n";
    out << "                    } else {\n";
    out << "                        column++;\n";
    out << "                    }\n";
    out << "                }\n";
    out << "                pos += token_text.length();\n";
    out << "                matched = true;\n";
    out << "                break;\n";
    out << "            }\n";
    out << "        }\n";
    out << "        \n";
    out << "        if (!matched) {\n";
    out << "            error(\"Unexpected character: '\" + std::string(1, input[pos]) + \"'\", line, column);\n";
    out << "            pos++;\n";
    out << "            column++;\n";
    out << "        }\n";
    out << "    }\n";
    out << "    \n";
    out << "    // Add EOF token\n";
    out << "    tokens.emplace_back(TokenType::EOF_TOKEN, \"\", line, column);\n";
    out << "    \n";
    out << "    return tokens;\n";
    out << "}\n\n";
    
    // Error handling
    out << "void " << base_name << "Lexer::error(const std::string& message, size_t line, size_t column) {\n";
    out << "    std::string error_msg = \"Lexer error at line \" + std::to_string(line) + \n";
    out << "                            \", column \" + std::to_string(column) + \": \" + message;\n";
    out << "    errors_.push_back(error_msg);\n";
    out << "    \n";
    out << "    if (verbose_) {\n";
    out << "        std::cerr << error_msg << std::endl;\n";
    out << "    }\n";
    out << "}\n\n";
    
    write_namespace_end(out);
    
    if (verbose_) {
        std::cout << "Generated lexer source: " << filename << std::endl;
    }
}

void CppCodeGenerator::generate_test_case(const std::string& filename, const std::string& base_name) {
    std::ofstream out(filename);
    if (!out) {
        throw std::runtime_error("Cannot create test file: " + filename);
    }
    
    write_file_header(out, "Generated Test Case for " + base_name + " Parser");
    
    // 包含必要的头文件
    out << "#include <iostream>\n";
    out << "#include <string>\n";
    out << "#include <vector>\n";
    out << "#include <cassert>\n";
    out << "#include <sstream>\n";
    out << "#include \"" << base_name << "Parser.h\"\n";
    out << "#include \"" << base_name << "Lexer.h\"\n";
    out << "#include \"" << base_name << "Tokens.h\"\n\n";
    
    write_namespace_begin(out);
    
    // 生成测试函数
    out << "// Test case function\n";
    out << "void run_test_case(const std::string& input, bool should_succeed = true) {\n";
    out << "    std::cout << \"Testing input: \\\"\" << input << \"\\\"\" << std::endl;\n";
    out << "    \n";
    out << "    try {\n";
    out << "        // Create lexer and tokenize input\n";
    out << "        " << base_name << "Lexer lexer;\n";
    out << "        auto tokens = lexer.tokenize(input);\n";
    out << "        \n";
    out << "        // Check for lexer errors\n";
    out << "        if (!lexer.get_errors().empty()) {\n";
    out << "            if (should_succeed) {\n";
    out << "                std::cout << \"  ✗ FAILED: Lexer errors: \";\n";
    out << "                for (const auto& error : lexer.get_errors()) {\n";
    out << "                    std::cout << error << \"; \";\n";
    out << "                }\n";
    out << "                std::cout << std::endl;\n";
    out << "                return;\n";
    out << "            }\n";
    out << "        }\n";
    out << "        \n";
    out << "        // Create parser\n";
    out << "        " << class_name_ << " parser;\n";
    out << "        \n";
    out << "        // Parse\n";
    out << "        auto result = parser.parse(tokens);\n";
    out << "        \n";
    out << "        if (should_succeed) {\n";
    out << "            if (result) {\n";
    out << "                std::cout << \"  ✓ PASSED: Successfully parsed\" << std::endl;\n";
    out << "            } else {\n";
    out << "                std::cout << \"  ✗ FAILED: Expected success but got null result\" << std::endl;\n";
    out << "            }\n";
    out << "        } else {\n";
    out << "            if (!result) {\n";
    out << "                std::cout << \"  ✓ PASSED: Correctly rejected invalid input\" << std::endl;\n";
    out << "            } else {\n";
    out << "                std::cout << \"  ✗ FAILED: Expected failure but parsing succeeded\" << std::endl;\n";
    out << "            }\n";
    out << "        }\n";
    out << "    } catch (const std::exception& e) {\n";
    out << "        if (should_succeed) {\n";
    out << "            std::cout << \"  ✗ FAILED: Exception thrown: \" << e.what() << std::endl;\n";
    out << "        } else {\n";
    out << "            std::cout << \"  ✓ PASSED: Correctly threw exception: \" << e.what() << std::endl;\n";
    out << "        }\n";
    out << "    }\n";
    out << "    \n";
    out << "    std::cout << std::endl;\n";
    out << "}\n\n";
    
    write_namespace_end(out);
    
    // 生成主函数
    out << "int main() {\n";
    out << "    using namespace " << namespace_ << ";\n";
    out << "    \n";
    out << "    std::cout << \"Running test cases for " << base_name << " parser...\" << std::endl;\n";
    out << "    std::cout << \"=========================================\" << std::endl;\n";
    out << "    std::cout << std::endl;\n";
    
    // 生成基本测试用例
    out << "    // Basic test cases\n";
    
    // 根据语法的起始符号生成测试用例
    auto start_symbol = grammar_.start_symbol();
    if (start_symbol) {
        // 生成简单的正向测试用例
        out << "    // TODO: Add specific test cases based on your grammar\n";
        out << "    // Example test cases (modify according to your grammar):\n";
        out << "    \n";
        
        // 根据产生式生成示例
        auto productions = grammar_.productions();
        if (!productions.empty()) {
            out << "    // Test case based on first production: " << productions[0]->to_string() << "\n";
            out << "    run_test_case(\"1+2*(3+4)\", true);\n";
            out << "    \n";
        }
        
        out << "    // Test empty input\n";
        out << "    run_test_case(\"\", false);\n";
        out << "    \n";
        out << "    // Test invalid input\n";
        out << "    run_test_case(\"invalid tokens here\", false);\n";
    }
    
    out << "    \n";
    out << "    std::cout << \"Test execution completed.\" << std::endl;\n";
    out << "    return 0;\n";
    out << "}\n\n";
    
    // write_namespace_end(out);
    
    if (verbose_) {
        std::cout << "Generated test case: " << filename << std::endl;
    }
}

} // namespace lalr1
