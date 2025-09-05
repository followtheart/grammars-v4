#include "g4_parser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <regex>

namespace lalr1 {

G4Parser::G4Parser() : verbose_(false) {
}

G4Parser::~G4Parser() {
}

std::unique_ptr<Grammar> G4Parser::parse_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        add_error("Cannot open file: " + filename);
        return nullptr;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return parse_string(buffer.str());
}

std::unique_ptr<Grammar> G4Parser::parse_string(const std::string& content) {
    errors_.clear();
    parser_rules_.clear();
    lexer_rules_.clear();
    
    if (verbose_) {
        std::cout << "Parsing ANTLR4 grammar..." << std::endl;
    }
    
    // 移除注释
    std::string clean_content = remove_comments(content);
    
    // 解析语法声明
    parse_grammar_declaration(clean_content);
    
    // 解析规则
    parse_rules(clean_content);
    
    if (!errors_.empty()) {
        return nullptr;
    }
    
    // 转换为Grammar对象
    return convert_to_grammar();
}

void G4Parser::parse_grammar_declaration(const std::string& content) {
    std::regex grammar_regex(R"(grammar\s+(\w+)\s*;)");
    std::smatch match;
    
    if (std::regex_search(content, match, grammar_regex)) {
        grammar_name_ = match[1].str();
        if (verbose_) {
            std::cout << "Found grammar: " << grammar_name_ << std::endl;
        }
    } else {
        add_error("No grammar declaration found");
    }
}

void G4Parser::parse_rules(const std::string& content) {
    // 预处理：将多行规则合并为单行
    std::string processed_content = content;
    
    // 匹配规则: rule_name : alternatives ;
    // 使用 [\s\S]*? 来匹配包括换行符在内的任何字符
    std::regex rule_regex(R"((\w+)\s*:\s*([\s\S]*?)\s*;)", std::regex_constants::ECMAScript);
    
    std::string::const_iterator searchStart(processed_content.cbegin());
    std::smatch match;
    
    while (std::regex_search(searchStart, processed_content.cend(), match, rule_regex)) {
        std::string rule_name = match[1].str();
        std::string rule_body = match[2].str();
        
        // 清理规则体 - 移除多余的空白和换行
        rule_body = std::regex_replace(rule_body, std::regex(R"(\s+)"), " ");
        rule_body = normalize_whitespace(rule_body);
        
        if (verbose_) {
            std::cout << "Found rule: " << rule_name << " : " << rule_body << std::endl;
        }
        
        if (is_lexer_rule_name(rule_name)) {
            parse_lexer_rule(rule_name + " : " + rule_body);
        } else {
            parse_parser_rule(rule_name + " : " + rule_body);
        }
        
        searchStart = match.suffix().first;
    }
    
    if (verbose_) {
        std::cout << "Parsed " << parser_rules_.size() << " parser rules and " 
                  << lexer_rules_.size() << " lexer rules" << std::endl;
    }
}

void G4Parser::parse_parser_rule(const std::string& rule_text) {
    std::regex rule_regex(R"((\w+)\s*:\s*(.*))");
    std::smatch match;
    
    if (!std::regex_match(rule_text, match, rule_regex)) {
        add_error("Invalid parser rule format: " + rule_text);
        return;
    }
    
    G4Rule rule;
    rule.name = match[1].str();
    rule.is_lexer_rule = false;
    
    std::string alternatives_text = match[2].str();
    rule.alternatives = split_alternatives(alternatives_text);
    
    parser_rules_.push_back(rule);
}

void G4Parser::parse_lexer_rule(const std::string& rule_text) {
    std::regex rule_regex(R"((\w+)\s*:\s*(.*))");
    std::smatch match;
    
    if (!std::regex_match(rule_text, match, rule_regex)) {
        add_error("Invalid lexer rule format: " + rule_text);
        return;
    }
    
    G4Token token;
    token.name = match[1].str();
    token.pattern = normalize_whitespace(match[2].str());
    token.is_fragment = token.name.find("fragment") == 0;
    token.is_skip = token.pattern.find("-> skip") != std::string::npos;
    
    lexer_rules_.push_back(token);
}

std::unique_ptr<Grammar> G4Parser::convert_to_grammar() {
    auto grammar = std::make_unique<Grammar>();
    
    if (verbose_) {
        std::cout << "Converting to internal grammar representation..." << std::endl;
    }
    
    // 转换语法规则
    convert_parser_rules(*grammar);
    
    // 转换词法规则 (在实际的LALR(1)解析器中，词法规则通常由独立的词法分析器处理)
    convert_lexer_rules(*grammar);
    
    // 设置开始符号为第一个解析规则
    if (!parser_rules_.empty()) {
        auto start_symbol = grammar->symbol_table().get_nonterminal(parser_rules_[0].name);
        grammar->set_start_symbol(start_symbol);
    }
    
    return grammar;
}

void G4Parser::convert_parser_rules(Grammar& grammar) {
    for (const auto& rule : parser_rules_) {
        auto lhs = grammar.symbol_table().get_nonterminal(rule.name);
        
        for (const auto& alt : rule.alternatives) {
            // 展开EBNF语法糖
            auto expanded = expand_ebnf(rule.name, alt);
            
            for (const auto& production_str : expanded) {
                // 解析产生式右侧
                std::istringstream iss(production_str);
                std::string token;
                std::vector<SymbolPtr> rhs;
                
                while (iss >> token) {
                    if (token == "EOF") {
                        continue; // 跳过EOF
                    }
                    
                    SymbolPtr symbol;
                    if (is_lexer_rule_name(token)) {
                        symbol = grammar.symbol_table().get_terminal(token, TokenType::IDENTIFIER);
                    } else if (token.front() == '\'' && token.back() == '\'') {
                        // 字符串字面量
                        std::string literal = token.substr(1, token.length() - 2);
                        symbol = grammar.symbol_table().get_terminal(literal, TokenType::IDENTIFIER);
                    } else {
                        symbol = grammar.symbol_table().get_nonterminal(token);
                    }
                    rhs.push_back(symbol);
                }
                
                if (rhs.empty()) {
                    // epsilon产生式
                    auto epsilon = grammar.symbol_table().get_epsilon();
                    grammar.add_production(lhs, {epsilon});
                } else {
                    grammar.add_production(lhs, rhs);
                }
            }
        }
    }
}

void G4Parser::convert_lexer_rules(Grammar& grammar) {
    // 在这个简化版本中，我们主要关注语法结构
    // 词法规则的详细处理可以后续完善
    for (const auto& token : lexer_rules_) {
        if (!token.is_fragment && !token.is_skip) {
            grammar.symbol_table().get_terminal(token.name, TokenType::IDENTIFIER);
        }
    }
}

std::vector<std::string> G4Parser::expand_ebnf(const std::string& rule_name, const std::string& alternative) {
    std::vector<std::string> result;
    
    // 简化的EBNF展开 - 在这个版本中，我们主要处理基本情况
    // 更复杂的EBNF展开需要更详细的解析
    
    std::string cleaned = normalize_whitespace(alternative);
    
    // 处理简单的选择 (用|分隔)
    if (cleaned.find('|') != std::string::npos) {
        auto alternatives = split_alternatives(cleaned);
        for (const auto& alt : alternatives) {
            result.push_back(normalize_whitespace(alt));
        }
    } else {
        result.push_back(cleaned);
    }
    
    return result;
}

std::string G4Parser::remove_comments(const std::string& content) {
    std::string result = content;
    
    // 移除单行注释 //
    std::regex single_comment(R"(//.*?$)", std::regex_constants::ECMAScript);
    result = std::regex_replace(result, single_comment, "");
    
    // 移除多行注释 /* */
    std::regex multi_comment(R"(/\*.*?\*/)", std::regex_constants::ECMAScript);
    result = std::regex_replace(result, multi_comment, "");
    
    // 移除文档注释 /** */
    std::regex doc_comment(R"(/\*\*.*?\*/)", std::regex_constants::ECMAScript);
    result = std::regex_replace(result, doc_comment, "");
    
    return result;
}

std::string G4Parser::normalize_whitespace(const std::string& text) {
    std::regex ws_regex(R"(\s+)");
    return std::regex_replace(text, ws_regex, " ");
}

std::vector<std::string> G4Parser::split_alternatives(const std::string& alternatives_text) {
    std::vector<std::string> alternatives;
    std::string current_alt;
    int paren_count = 0;
    
    for (char c : alternatives_text) {
        if (c == '(' || c == '[' || c == '{') {
            paren_count++;
            current_alt += c;
        } else if (c == ')' || c == ']' || c == '}') {
            paren_count--;
            current_alt += c;
        } else if (c == '|' && paren_count == 0) {
            alternatives.push_back(normalize_whitespace(current_alt));
            current_alt.clear();
        } else {
            current_alt += c;
        }
    }
    
    if (!current_alt.empty()) {
        alternatives.push_back(normalize_whitespace(current_alt));
    }
    
    return alternatives;
}

bool G4Parser::is_lexer_rule_name(const std::string& name) {
    return !name.empty() && std::isupper(name[0]);
}

void G4Parser::add_error(const std::string& message) {
    errors_.push_back(message);
    if (verbose_) {
        std::cerr << "Error: " << message << std::endl;
    }
}

// G4Utils 实现
bool G4Utils::is_lalr1_grammar(const std::string& grammar_file) {
    try {
        G4Parser parser;
        parser.set_verbose(false);
        
        auto grammar = parser.parse_file(grammar_file);
        if (!grammar) {
            return false;
        }
        
        // 这里可以添加更详细的LALR(1)检查逻辑
        return parser.get_errors().empty();
        
    } catch (const std::exception& e) {
        return false;
    }
}

bool G4Utils::convert_to_bnf(const std::string& input_file, const std::string& output_file) {
    try {
        G4Parser parser;
        parser.set_verbose(true);
        
        auto grammar = parser.parse_file(input_file);
        if (!grammar) {
            return false;
        }
        
        std::ofstream out(output_file);
        if (!out.is_open()) {
            return false;
        }
        
        out << "# BNF Grammar converted from " << input_file << std::endl;
        out << std::endl;
        
        grammar->print_grammar_to_stream(out);
        out.close();
        
        return true;
        
    } catch (const std::exception& e) {
        return false;
    }
}

void G4Utils::analyze_grammar(const std::string& grammar_file) {
    G4Parser parser;
    parser.set_verbose(true);
    
    std::cout << "Analyzing ANTLR4 grammar: " << grammar_file << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    auto grammar = parser.parse_file(grammar_file);
    if (!grammar) {
        std::cout << "Failed to parse grammar file." << std::endl;
        for (const auto& error : parser.get_errors()) {
            std::cout << "Error: " << error << std::endl;
        }
        return;
    }
    
    std::cout << "Grammar analysis completed successfully." << std::endl;
    std::cout << "Productions: " << grammar->productions().size() << std::endl;
    std::cout << "Nonterminals: " << grammar->symbol_table().get_nonterminals().size() << std::endl;
    std::cout << "Terminals: " << grammar->symbol_table().get_terminals().size() << std::endl;
}

} // namespace lalr1
