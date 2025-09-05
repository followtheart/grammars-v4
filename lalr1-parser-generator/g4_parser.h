#pragma once

#include "grammar.h"
#include "lexer.h"
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <regex>

namespace lalr1 {

/**
 * ANTLR4 .g4语法文件解析器
 * 将ANTLR4语法转换为LALR(1)内部表示
 */
class G4Parser {
public:
    G4Parser();
    ~G4Parser();
    
    /**
     * 从.g4文件解析语法
     * @param filename .g4文件路径
     * @return 解析得到的Grammar对象
     */
    std::unique_ptr<Grammar> parse_file(const std::string& filename);
    
    /**
     * 从字符串解析语法
     * @param content .g4文件内容
     * @return 解析得到的Grammar对象
     */
    std::unique_ptr<Grammar> parse_string(const std::string& content);
    
    /**
     * 设置是否启用详细输出
     */
    void set_verbose(bool verbose) { verbose_ = verbose; }
    
    /**
     * 获取解析过程中的错误信息
     */
    const std::vector<std::string>& get_errors() const { return errors_; }
    
private:
    struct G4Rule {
        std::string name;
        std::vector<std::string> alternatives;
        bool is_lexer_rule;  // true为词法规则，false为语法规则
    };
    
    struct G4Token {
        std::string name;
        std::string pattern;
        bool is_fragment;
        bool is_skip;
    };
    
    bool verbose_;
    std::vector<std::string> errors_;
    std::string grammar_name_;
    std::vector<G4Rule> parser_rules_;
    std::vector<G4Token> lexer_rules_;
    
    // 解析步骤
    void parse_grammar_declaration(const std::string& content);
    void parse_rules(const std::string& content);
    void parse_parser_rule(const std::string& rule_text);
    void parse_lexer_rule(const std::string& rule_text);
    
    // 语法转换
    std::unique_ptr<Grammar> convert_to_grammar();
    void convert_parser_rules(Grammar& grammar);
    void convert_lexer_rules(Grammar& grammar);
    
    // EBNF转换辅助函数
    std::vector<std::string> expand_ebnf(const std::string& rule_name, const std::string& alternative);
    std::string handle_repetition(const std::string& base_name, const std::string& element);
    std::string handle_optional(const std::string& base_name, const std::string& element);
    std::string handle_grouping(const std::string& base_name, const std::string& group);
    
    // 工具函数
    std::string remove_comments(const std::string& content);
    std::string normalize_whitespace(const std::string& text);
    std::vector<std::string> split_alternatives(const std::string& alternatives_text);
    std::string escape_regex(const std::string& text);
    bool is_lexer_rule_name(const std::string& name);
    void add_error(const std::string& message);
};

/**
 * G4语法工具函数
 */
class G4Utils {
public:
    /**
     * 验证G4语法文件是否为LALR(1)
     * @param grammar_file .g4文件路径
     * @return true如果是LALR(1)语法
     */
    static bool is_lalr1_grammar(const std::string& grammar_file);
    
    /**
     * 将G4语法转换为传统BNF格式
     * @param input_file 输入.g4文件
     * @param output_file 输出.bnf文件
     * @return true如果转换成功
     */
    static bool convert_to_bnf(const std::string& input_file, const std::string& output_file);
    
    /**
     * 分析G4语法的复杂度和特性
     * @param grammar_file .g4文件路径
     */
    static void analyze_grammar(const std::string& grammar_file);
};

} // namespace lalr1
