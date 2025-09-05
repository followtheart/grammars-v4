#pragma once

#include "token.h"
#include <string>
#include <vector>
#include <memory>
#include <regex>
#include <functional>

namespace lalr1 {

// Token rule for lexical analysis
struct TokenRule {
    TokenType type;
    std::string pattern;  // Regex pattern
    std::regex regex;
    bool skip;  // Whether to skip this token type (e.g., whitespace)
    
    TokenRule(TokenType t, const std::string& p, bool s = false)
        : type(t), pattern(p), regex(p), skip(s) {}
};

// Lexical analyzer
class Lexer {
public:
    Lexer();
    ~Lexer();
    
    // Add token rules
    void add_rule(TokenType type, const std::string& pattern, bool skip = false);
    void add_keyword(const std::string& keyword, TokenType type);
    
    // Set input
    void set_input(const std::string& input);
    
    // Get next token
    Token next_token();
    
    // Peek at next token without consuming it
    Token peek_token();
    
    // Reset to beginning
    void reset();
    
    // Check if we're at end of input
    bool at_end() const;
    
    // Get current position information
    size_t get_line() const { return current_line_; }
    size_t get_column() const { return current_column_; }
    size_t get_position() const { return current_pos_; }
    
    // Error handling
    std::string get_error_message() const { return error_message_; }
    bool has_error() const { return !error_message_.empty(); }

private:
    std::vector<TokenRule> rules_;
    std::string input_;
    size_t current_pos_;
    size_t current_line_;
    size_t current_column_;
    std::string error_message_;
    
    // Helper methods
    char current_char() const;
    char peek_char(size_t offset = 1) const;
    void advance();
    void skip_whitespace();
    Token match_token();
    void update_position(char c);
};

// Default lexer for calculator expressions
std::unique_ptr<Lexer> create_calculator_lexer();

} // namespace lalr1