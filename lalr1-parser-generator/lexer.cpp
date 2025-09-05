#include "lexer.h"
#include <iostream>
#include <cctype>

namespace lalr1 {

Lexer::Lexer() : current_pos_(0), current_line_(1), current_column_(1) {
}

Lexer::~Lexer() {
}

void Lexer::add_rule(TokenType type, const std::string& pattern, bool skip) {
    try {
        rules_.emplace_back(type, pattern, skip);
    } catch (const std::regex_error& e) {
        error_message_ = "Invalid regex pattern: " + pattern + " - " + e.what();
    }
}

void Lexer::add_keyword(const std::string& keyword, TokenType type) {
    // Escape special regex characters and create exact match pattern
    std::string escaped_keyword;
    for (char c : keyword) {
        if (c == '.' || c == '^' || c == '$' || c == '*' || c == '+' || 
            c == '?' || c == '(' || c == ')' || c == '[' || c == ']' ||
            c == '{' || c == '}' || c == '|' || c == '\\') {
            escaped_keyword += '\\';
        }
        escaped_keyword += c;
    }
    
    add_rule(type, "^" + escaped_keyword + "$");
}

void Lexer::set_input(const std::string& input) {
    input_ = input;
    reset();
}

void Lexer::reset() {
    current_pos_ = 0;
    current_line_ = 1;
    current_column_ = 1;
    error_message_.clear();
}

bool Lexer::at_end() const {
    return current_pos_ >= input_.length();
}

char Lexer::current_char() const {
    if (at_end()) return '\0';
    return input_[current_pos_];
}

char Lexer::peek_char(size_t offset) const {
    size_t pos = current_pos_ + offset;
    if (pos >= input_.length()) return '\0';
    return input_[pos];
}

void Lexer::advance() {
    if (!at_end()) {
        update_position(current_char());
        current_pos_++;
    }
}

void Lexer::update_position(char c) {
    if (c == '\n') {
        current_line_++;
        current_column_ = 1;
    } else {
        current_column_++;
    }
}

void Lexer::skip_whitespace() {
    while (!at_end() && std::isspace(current_char())) {
        advance();
    }
}

Token Lexer::next_token() {
    if (at_end()) {
        return Token(TokenType::EOF_TOKEN, "", current_line_, current_column_, current_pos_);
    }
    
    // Try to match against all rules
    for (const auto& rule : rules_) {
        std::string remaining = input_.substr(current_pos_);
        std::smatch match;
        
        if (std::regex_search(remaining, match, rule.regex) && match.position() == 0) {
            std::string matched_text = match.str();
            size_t start_line = current_line_;
            size_t start_column = current_column_;
            size_t start_pos = current_pos_;
            
            // Advance past the matched text
            for (char c : matched_text) {
                update_position(c);
                current_pos_++;
            }
            
            if (rule.skip) {
                return next_token();  // Skip this token and get the next one
            }
            
            return Token(rule.type, matched_text, start_line, start_column, start_pos);
        }
    }
    
    // No rule matched - create error token
    char error_char = current_char();
    std::string error_text(1, error_char);
    size_t start_line = current_line_;
    size_t start_column = current_column_;
    size_t start_pos = current_pos_;
    
    advance();
    
    error_message_ = "Unexpected character: '" + error_text + "' at line " + 
                    std::to_string(start_line) + ", column " + std::to_string(start_column);
    
    return Token(TokenType::ERROR_TOKEN, error_text, start_line, start_column, start_pos);
}

Token Lexer::peek_token() {
    // Save current state
    size_t saved_pos = current_pos_;
    size_t saved_line = current_line_;
    size_t saved_column = current_column_;
    std::string saved_error = error_message_;
    
    // Get next token
    Token token = next_token();
    
    // Restore state
    current_pos_ = saved_pos;
    current_line_ = saved_line;
    current_column_ = saved_column;
    error_message_ = saved_error;
    
    return token;
}

Token Lexer::match_token() {
    return next_token();
}

std::unique_ptr<Lexer> create_calculator_lexer() {
    auto lexer = std::make_unique<Lexer>();
    
    // Skip whitespace
    lexer->add_rule(TokenType::WHITESPACE, "[ \t]+", true);
    lexer->add_rule(TokenType::NEWLINE, "\n", true);
    
    // Numbers (integers and floats)
    lexer->add_rule(TokenType::NUMBER, "[0-9]+\\.?[0-9]*");
    
    // Identifiers (variables and functions)
    lexer->add_rule(TokenType::IDENTIFIER, "[a-zA-Z][a-zA-Z0-9]*");
    
    // Operators
    lexer->add_rule(TokenType::PLUS, "\\+");
    lexer->add_rule(TokenType::MINUS, "-");
    lexer->add_rule(TokenType::MULTIPLY, "\\*");
    lexer->add_rule(TokenType::DIVIDE, "/");
    lexer->add_rule(TokenType::POWER, "\\^");
    
    // Comparison operators
    lexer->add_rule(TokenType::EQUAL, "=");
    lexer->add_rule(TokenType::LESS_THAN, "<");
    lexer->add_rule(TokenType::GREATER_THAN, ">");
    
    // Delimiters
    lexer->add_rule(TokenType::LPAREN, "\\(");
    lexer->add_rule(TokenType::RPAREN, "\\)");
    lexer->add_rule(TokenType::COMMA, ",");
    lexer->add_rule(TokenType::SEMICOLON, ";");
    
    return lexer;
}

} // namespace lalr1