#pragma once

#include <string>
#include <memory>

namespace lalr1 {

// Token types
enum class TokenType {
    // Special tokens
    EOF_TOKEN = 0,
    ERROR_TOKEN,
    
    // Basic tokens
    IDENTIFIER,
    NUMBER,
    STRING,
    
    // Operators
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    POWER,
    ASSIGN,
    
    // Comparison operators
    EQUAL,
    NOT_EQUAL,
    LESS_THAN,
    GREATER_THAN,
    LESS_EQUAL,
    GREATER_EQUAL,
    
    // Delimiters
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    SEMICOLON,
    COMMA,
    
    // Whitespace
    WHITESPACE,
    NEWLINE,
    
    // Custom tokens (can be extended)
    CUSTOM_START = 1000
};

// Token structure
struct Token {
    TokenType type;
    std::string value;
    size_t line;
    size_t column;
    size_t position;
    
    Token() : type(TokenType::EOF_TOKEN), line(1), column(1), position(0) {}
    
    Token(TokenType t, const std::string& v, size_t l = 1, size_t c = 1, size_t p = 0)
        : type(t), value(v), line(l), column(c), position(p) {}
    
    bool is_eof() const { return type == TokenType::EOF_TOKEN; }
    bool is_error() const { return type == TokenType::ERROR_TOKEN; }
    
    std::string to_string() const;
};

// Token type utilities
std::string token_type_to_string(TokenType type);
TokenType string_to_token_type(const std::string& str);

} // namespace lalr1