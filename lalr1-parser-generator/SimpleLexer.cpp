#include "SimpleLexer.h"
#include <iostream>
#include <sstream>

namespace generated {

SimpleLexer::SimpleLexer(const std::string& input) 
    : input_(input), pos_(0), line_(1), column_(1) {
    init_patterns();
}

std::vector<Token> SimpleLexer::tokenize() {
    std::vector<Token> tokens;
    pos_ = 0;
    line_ = 1;
    column_ = 1;
    errors_.clear();

    while (pos_ < input_.size()) {
        skip_whitespace();
        if (pos_ >= input_.size()) break;

        Token token;
        if (match_token(token)) {
            tokens.push_back(token);
        } else {
            error("Unexpected character: " + std::string(1, input_[pos_]));
            pos_++;
            column_++;
        }
    }

    // Add EOF token
    tokens.emplace_back(TokenType::EOF_TOKEN, "", line_, column_);
    
    return tokens;
}

void SimpleLexer::init_patterns() {
    patterns_ = {
        {TokenType::STRING, R"("([^"\\]|\\.)*")"},
        {TokenType::NUMBER, R"(-?(?:0|[1-9]\d*)(?:\.\d+)?(?:[eE][+-]?\d+)?)"},
        {TokenType::TRUE, R"(true)"},
        {TokenType::FALSE, R"(false)"},
        {TokenType::NULL_TOKEN, R"(null)"},
        {TokenType::LBRACE, R"(\{)"},
        {TokenType::RBRACE, R"(\})"},
        {TokenType::LBRACKET, R"(\[)"},
        {TokenType::RBRACKET, R"(\])"},
        {TokenType::COMMA, R"(,)"},
        {TokenType::COLON, R"(:)"}
    };
}

bool SimpleLexer::match_token(Token& token) {
    for (const auto& pattern : patterns_) {
        std::smatch match;
        std::string remaining = input_.substr(pos_);
        
        if (std::regex_search(remaining, match, pattern.pattern) && match.position() == 0) {
            token.type = pattern.type;
            token.text = match.str();
            token.line = line_;
            token.column = column_;
            
            pos_ += match.length();
            column_ += match.length();
            
            return true;
        }
    }
    return false;
}

void SimpleLexer::skip_whitespace() {
    while (pos_ < input_.size() && std::isspace(input_[pos_])) {
        if (input_[pos_] == '\n') {
            line_++;
            column_ = 1;
        } else {
            column_++;
        }
        pos_++;
    }
}

void SimpleLexer::error(const std::string& message) {
    std::ostringstream oss;
    oss << "Lexer error at line " << line_ << ", column " << column_ << ": " << message;
    errors_.push_back(oss.str());
}

} // namespace generated
