#pragma once

#include "grammar.h"
#include "parse_table.h"
#include "g4_parser.h"
#include <string>
#include <fstream>
#include <memory>

namespace lalr1 {

/**
 * C++代码生成器
 * 根据LALR(1)语法和解析表生成完整的C++解析器
 */
class CppCodeGenerator {
public:
    CppCodeGenerator(const Grammar& grammar, std::shared_ptr<ParseTable> table);
    ~CppCodeGenerator();
    
    /**
     * 生成完整的C++解析器
     * @param base_name 生成文件的基础名称
     * @param output_dir 输出目录
     * @return 是否生成成功
     */
    bool generate_parser(const std::string& base_name, const std::string& output_dir = ".");
    
    /**
     * 设置生成选项
     */
    void set_namespace(const std::string& ns) { namespace_ = ns; }
    void set_class_name(const std::string& name) { class_name_ = name; }
    void set_generate_visitor(bool generate) { generate_visitor_ = generate; }
    void set_generate_listener(bool generate) { generate_listener_ = generate; }
    void set_verbose(bool verbose) { verbose_ = verbose; }
    
private:
    const Grammar& grammar_;
    std::shared_ptr<ParseTable> table_;
    std::string namespace_;
    std::string class_name_;
    bool generate_visitor_;
    bool generate_listener_;
    bool verbose_;
    
    // 代码生成辅助函数
    void generate_header_file(const std::string& filename, const std::string& base_name);
    void generate_source_file(const std::string& filename, const std::string& base_name);
    void generate_token_header(const std::string& filename, const std::string& base_name);
    void generate_ast_nodes(const std::string& filename, const std::string& base_name);
    void generate_lexer_header(const std::string& filename, const std::string& base_name);
    void generate_lexer_source(const std::string& filename, const std::string& base_name);
    void generate_visitor_pattern(const std::string& filename, const std::string& base_name);
    void generate_listener_pattern(const std::string& filename, const std::string& base_name);
    void generate_test_case(const std::string& filename, const std::string& base_name);
    
    // 具体生成函数
    void write_file_header(std::ofstream& out, const std::string& description);
    void write_includes(std::ofstream& out, bool is_header = true);
    void write_namespace_begin(std::ofstream& out);
    void write_namespace_end(std::ofstream& out);
    void write_token_definitions(std::ofstream& out);
    void write_ast_node_classes(std::ofstream& out);
    void write_parser_class_declaration(std::ofstream& out);
    void write_parser_class_implementation(std::ofstream& out);
    void write_parsing_methods(std::ofstream& out);
    void write_action_table(std::ofstream& out);
    void write_goto_table(std::ofstream& out);
    void write_production_rules(std::ofstream& out);
    void write_error_handling(std::ofstream& out);
    
    // 工具函数
    std::string to_cpp_identifier(const std::string& name);
    std::string to_token_name(const std::string& name);
    std::string to_class_name(const std::string& name);
    std::string get_production_method_name(const ProductionPtr& production);
    std::string get_ast_node_name(const std::string& rule_name);
    void write_indent(std::ofstream& out, int level);
};

/**
 * Makefile/CMake生成器
 */
class BuildFileGenerator {
public:
    static bool generate_makefile(const std::string& base_name, const std::string& output_dir = ".");
    static bool generate_cmake(const std::string& base_name, const std::string& output_dir = ".");
};

} // namespace lalr1
