#pragma once

#include <string>
#include <vector>
#include <regex>
#include <memory>

namespace generated {

enum class TokenType {
    EOF_TOKEN = 0,
    STRING = 1,
    NUMBER = 2,
    TRUE = 3,
    FALSE = 4,
    NULL_TOKEN = 5,
    LBRACE = 6,    // {
    RBRACE = 7,    // }
    LBRACKET = 8,  // [
    RBRACKET = 9,  // ]
    COMMA = 10,    // ,
    COLON = 11,    // :
};

struct Token {
    TokenType type;
    std::string text;
    size_t line;
    size_t column;

    Token(TokenType t = TokenType::EOF_TOKEN, const std::string& txt = "", size_t ln = 0, size_t col = 0)
        : type(t), text(txt), line(ln), column(col) {}
};

/**
 * Simple lexer for JSON tokens
 */
class SimpleLexer {
public:
    SimpleLexer(const std::string& input);
    
    std::vector<Token> tokenize();
    const std::vector<std::string>& get_errors() const { return errors_; }

private:
    struct TokenPattern {
        TokenType type;
        std::regex pattern;
        TokenPattern(TokenType t, const std::string& p) : type(t), pattern(p) {}
    };

    std::string input_;
    size_t pos_;
    size_t line_;
    size_t column_;
    std::vector<std::string> errors_;
    std::vector<TokenPattern> patterns_;

    void init_patterns();
    bool match_token(Token& token);
    void skip_whitespace();
    void error(const std::string& message);
};

} // namespace generated
