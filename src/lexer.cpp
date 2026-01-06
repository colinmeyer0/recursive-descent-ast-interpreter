#include "lexer.hpp"

#include <cctype>
#include <unordered_map>

Lexer::Lexer(std::string source): source_(std::move(source)) {} // Store source buffer

std::vector<Token> Lexer::scan_tokens() {
    while (!is_at_end()) {
        start_ = current_; // Mark beginning of next lexeme
        start_pos_ = SourcePos{line_, col_}; // Remember line/col for this token
        scan_token(); // Consume one token (or skip whitespace/comments)
    }

    // Always append EOF to simplify parser logic
    Span span;
    span.start = current_; // EOF span is zero-length at end
    span.end = current_;
    span.pos = SourcePos{line_, col_};
    tokens_.push_back(
        Token{TokenType::EOF_, "", std::monostate{}, span}); // Sentinel

    return tokens_;
}

const std::vector<std::string> &Lexer::errors() const {
    return errors_;
} // Access errors

bool Lexer::is_at_end() const {
    return current_ >= source_.size();
} // Bounds check

char Lexer::advance() {
    char c = source_[current_++]; // Consume current char
    if (c == '\n') {
        line_++;  // Track newline
        col_ = 1; // Reset column to 1-based
    } else {
        col_++; // Advance column on same line
    }
    return c;
}

bool Lexer::match(char expected) {
    if (is_at_end() || source_[current_] != expected) {
        return false; // No match
    }
    advance(); // Consume matched char
    return true;
}

char Lexer::peek() const {
    return is_at_end() ? '\0' : source_[current_];
} // Lookahead

char Lexer::peek_next() const {
    if (current_ + 1 >= source_.size()) {
        return '\0'; // No next char
    }
    return source_[current_ + 1];
}

void Lexer::scan_token() {
    char c = advance(); // Consume next char to decide token type
    switch (c) {
    case '(':
        add_token(TokenType::LEFT_PAREN);
        break;
    case ')':
        add_token(TokenType::RIGHT_PAREN);
        break;
    case '{':
        add_token(TokenType::LEFT_BRACE);
        break;
    case '}':
        add_token(TokenType::RIGHT_BRACE);
        break;
    case ';':
        add_token(TokenType::SEMICOLON);
        break;
    case ',':
        add_token(TokenType::COMMA);
        break;
    case '+':
        add_token(TokenType::PLUS);
        break;
    case '-':
        add_token(TokenType::MINUS);
        break;
    case '*':
        add_token(TokenType::STAR);
        break;
    case '!':
        add_token(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
        break;
    case '=':
        add_token(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
        break;
    case '<':
        add_token(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
        break;
    case '>':
        add_token(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
        break;
    case '&':
        if (match('&')) {
            add_token(TokenType::AND_AND);
        } else {
            add_error("Unexpected '&' without pair.",
                      start_pos_); // Single '&' invalid
        }
        break;
    case '|':
        if (match('|')) {
            add_token(TokenType::OR_OR);
        } else {
            add_error("Unexpected '|' without pair.",
                      start_pos_); // Single '|' invalid
        }
        break;
    case '/':
        if (match('/')) {
            // Line comment: consume until newline or EOF
            while (peek() != '\n' && !is_at_end()) {
                advance(); // Discard comment text
            }
        } else {
            add_token(TokenType::SLASH);
        }
        break;
    case ' ':
    case '\r':
    case '\t':
    case '\n':
        break; // Ignore whitespace.
    default:
        if (is_digit(c)) {
            // Integer literals only for now.
            while (is_digit(peek())) {
                advance(); // Consume remaining digits.
            }
            std::string text =
                source_.substr(start_, current_ - start_); // Raw digits.
            int value = std::stoi(text); // Simple base-10 parse.
            add_token(TokenType::NUMBER, value);
        } else if (is_alpha(c)) {
            // Identifiers and keywords.
            while (is_alnum(peek())) {
                advance(); // Extend identifier.
            }
            std::string text =
                source_.substr(start_, current_ - start_); // Identifier text.
            static const std::unordered_map<std::string, TokenType> keywords = {
                {"let", TokenType::LET},
                {"if", TokenType::IF},
                {"else", TokenType::ELSE},
                {"while", TokenType::WHILE},
                {"break", TokenType::BREAK},
                {"continue", TokenType::CONTINUE},
                {"return", TokenType::RETURN},
                {"fn", TokenType::FN},
                {"true", TokenType::TRUE},
                {"false", TokenType::FALSE},
            };
            auto it = keywords.find(text); // Lookup keyword.
            if (it != keywords.end()) {
                TokenType type = it->second; // Keyword token type.
                if (type == TokenType::TRUE) {
                    add_token(type, true);
                } else if (type == TokenType::FALSE) {
                    add_token(type, false);
                } else {
                    add_token(type);
                }
            } else {
                add_token(TokenType::IDENTIFIER);
            }
        } else {
            add_error("Unexpected character.",
                      start_pos_); // Unrecognized input.
        }
        break;
    }
}

void Lexer::add_token(TokenType type) {
    add_token(type, std::monostate{});
} // No literal.

void Lexer::add_token(TokenType type, Literal literal) {
    Span span;
    span.start = start_;
    span.end = current_;
    span.pos = start_pos_;
    std::string lexeme =
        source_.substr(start_, current_ - start_); // Slice text.
    tokens_.push_back(
        Token{type, lexeme, std::move(literal), span}); // Emit token.
}

void Lexer::add_error(const std::string &message, const SourcePos &pos) {
    errors_.push_back("Line " + std::to_string(pos.line) + ", col " +
                      std::to_string(pos.col) + ": " +
                      message); // Format error.
}

bool Lexer::is_alpha(char c) const {
    return std::isalpha(static_cast<unsigned char>(c)) ||
           c == '_'; // ASCII-ish.
}

bool Lexer::is_digit(char c) const {
    return std::isdigit(static_cast<unsigned char>(c)); // ASCII digits only.
}

bool Lexer::is_alnum(char c) const {
    return is_alpha(c) || is_digit(c);
} // Combined.
