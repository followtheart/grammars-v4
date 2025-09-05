#include "token.h"
#include <sstream>
#include <unordered_map>

namespace lalr1 {

std::string Token::to_string() const {
    std::ostringstream oss;
    oss << "Token(" << token_type_to_string(type) << ", \"" << value 
        << "\", " << line << ":" << column << ")";
    return oss.str();
}

std::string token_type_to_string(TokenType type) {
    static const std::unordered_map<TokenType, std::string> type_map = {
        {TokenType::EOF_TOKEN, "EOF"},
        {TokenType::ERROR_TOKEN, "ERROR"},
        {TokenType::IDENTIFIER, "IDENTIFIER"},
        {TokenType::NUMBER, "NUMBER"},
        {TokenType::STRING, "STRING"},
        {TokenType::PLUS, "PLUS"},
        {TokenType::MINUS, "MINUS"},
        {TokenType::MULTIPLY, "MULTIPLY"},
        {TokenType::DIVIDE, "DIVIDE"},
        {TokenType::POWER, "POWER"},
        {TokenType::ASSIGN, "ASSIGN"},
        {TokenType::EQUAL, "EQUAL"},
        {TokenType::NOT_EQUAL, "NOT_EQUAL"},
        {TokenType::LESS_THAN, "LESS_THAN"},
        {TokenType::GREATER_THAN, "GREATER_THAN"},
        {TokenType::LESS_EQUAL, "LESS_EQUAL"},
        {TokenType::GREATER_EQUAL, "GREATER_EQUAL"},
        {TokenType::LPAREN, "LPAREN"},
        {TokenType::RPAREN, "RPAREN"},
        {TokenType::LBRACE, "LBRACE"},
        {TokenType::RBRACE, "RBRACE"},
        {TokenType::SEMICOLON, "SEMICOLON"},
        {TokenType::COMMA, "COMMA"},
        {TokenType::WHITESPACE, "WHITESPACE"},
        {TokenType::NEWLINE, "NEWLINE"}
    };
    
    auto it = type_map.find(type);
    if (it != type_map.end()) {
        return it->second;
    }
    
    return "UNKNOWN(" + std::to_string(static_cast<int>(type)) + ")";
}

TokenType string_to_token_type(const std::string& str) {
    static const std::unordered_map<std::string, TokenType> type_map = {
        {"EOF", TokenType::EOF_TOKEN},
        {"ERROR", TokenType::ERROR_TOKEN},
        {"IDENTIFIER", TokenType::IDENTIFIER},
        {"NUMBER", TokenType::NUMBER},
        {"STRING", TokenType::STRING},
        {"PLUS", TokenType::PLUS},
        {"MINUS", TokenType::MINUS},
        {"MULTIPLY", TokenType::MULTIPLY},
        {"DIVIDE", TokenType::DIVIDE},
        {"POWER", TokenType::POWER},
        {"ASSIGN", TokenType::ASSIGN},
        {"EQUAL", TokenType::EQUAL},
        {"NOT_EQUAL", TokenType::NOT_EQUAL},
        {"LESS_THAN", TokenType::LESS_THAN},
        {"GREATER_THAN", TokenType::GREATER_THAN},
        {"LESS_EQUAL", TokenType::LESS_EQUAL},
        {"GREATER_EQUAL", TokenType::GREATER_EQUAL},
        {"LPAREN", TokenType::LPAREN},
        {"RPAREN", TokenType::RPAREN},
        {"LBRACE", TokenType::LBRACE},
        {"RBRACE", TokenType::RBRACE},
        {"SEMICOLON", TokenType::SEMICOLON},
        {"COMMA", TokenType::COMMA},
        {"WHITESPACE", TokenType::WHITESPACE},
        {"NEWLINE", TokenType::NEWLINE}
    };
    
    auto it = type_map.find(str);
    if (it != type_map.end()) {
        return it->second;
    }
    
    return TokenType::ERROR_TOKEN;
}

} // namespace lalr1