#include <unordered_map>

#include "lexer.hpp"

Lexer::Lexer(std::string source) : source_(source) {} // Store source string

std::vector<Token> Lexer::scan_tokens() {
    while (!is_at_end()) {
        start_ = current_;                   // Mark beginning of next lexeme
        start_pos_ = SourcePos{line_, col_}; // Remember line/col for this token
        scan_token();                        // Consume one token (or skip whitespace/comments)
    }

    // Always append EOF to simplify parser logic
    start_ = current_;
    start_pos_ = SourcePos{line_, col_};
    add_token(TokenType::EOF_);

    return tokens_;
}

void Lexer::scan_token() {
    char c = advance();
    if (scan_single_char_token(c)) return;
    if (scan_two_char_operator(c)) return;
    if (scan_paired_operator(c)) return;
    if (scan_slash_or_comment(c)) return;
    if (is_whitespace(c)) return;

    if (is_digit(c)) {
        scan_number();
        return;
    }
    if (is_alpha(c)) {
        scan_identifier_or_keyword();
        return;
    }

    add_error("Unexpected character.", start_pos_);
}

bool Lexer::scan_single_char_token(char c) {
    switch (c) {
    case '(': add_token(TokenType::LEFT_PAREN); return true;
    case ')': add_token(TokenType::RIGHT_PAREN); return true;
    case '{': add_token(TokenType::LEFT_BRACE); return true;
    case '}': add_token(TokenType::RIGHT_BRACE); return true;
    case ';': add_token(TokenType::SEMICOLON); return true;
    case ',': add_token(TokenType::COMMA); return true;
    case '+': add_token(TokenType::PLUS); return true;
    case '-': add_token(TokenType::MINUS); return true;
    case '*': add_token(TokenType::STAR); return true;
    default: return false;
    }
}

bool Lexer::scan_two_char_operator(char c) {
    switch (c) {
    case '!': add_token(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG); return true;
    case '=': add_token(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL); return true;
    case '<': add_token(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS); return true;
    case '>': add_token(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); return true;
    default: return false;
    }
}

bool Lexer::scan_paired_operator(char c) {
    // check next char to see if paired, otherwise error
    switch (c) {
    case '&':
        if (match('&')) add_token(TokenType::AND_AND);
        else add_error("Unexpected '&' without pair.", start_pos_);
        return true;
    case '|':
        if (match('|')) add_token(TokenType::OR_OR);
        else add_error("Unexpected '|' without pair.", start_pos_);
        return true;
    default:
        return false;
    }
}

bool Lexer::scan_slash_or_comment(char c) {
    if (c != '/') return false;

    // peak next char to see if comment -> discard rest of line
    if (match('/')) {
        while (peek() != '\n' && !is_at_end()) advance();
    }
    else {
        add_token(TokenType::SLASH);
    }
    return true;
}

void Lexer::scan_number() {
    while (is_digit(peek()))
        advance(); // advance to end of number

    // add value of number to token vector
    std::string text = source_.substr(start_, current_ - start_);
    int value = std::stoi(text);
    add_token(TokenType::NUMBER, value);
}

void Lexer::scan_identifier_or_keyword() {
    while (is_alnum(peek()))
        advance(); // advance to end of identifier/keyword

    std::string text = source_.substr(start_, current_ - start_);

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

    auto it = keywords.find(text);

    // doesn't appear in keywords -> identifier
    if (it == keywords.end()) {
        add_token(TokenType::IDENTIFIER);
        return;
    }

    // add keyword token
    TokenType type = it->second;
    if (type == TokenType::TRUE) {
        add_token(type, true);
    }
    else if (type == TokenType::FALSE) {
        add_token(type, false);
    }
    else {
        add_token(type);
    }
}

void Lexer::add_token(TokenType type) {
    // no literal value
    add_token(type, std::monostate{});
}

void Lexer::add_token(TokenType type, Literal literal) {
    Span span;
    span.start = start_;
    span.end = current_;
    span.pos = start_pos_;
    std::string lexeme = source_.substr(start_, current_ - start_);   // slice text
    tokens_.push_back(Token{type, lexeme, std::move(literal), span}); // add token to token vector
}

void Lexer::add_error(const std::string &message, const SourcePos &pos) {
    errors_.push_back("Line " + std::to_string(pos.line) + ", col " +
                      std::to_string(pos.col) + ": " + message); // format error
}

const std::vector<std::string> &Lexer::errors() const {
    return errors_; // access errors
}

bool Lexer::is_whitespace(char c) const {
    return c == ' ' || c == '\r' || c == '\t' || c == '\n';
}

bool Lexer::is_at_end() const {
    return current_ >= source_.size(); // bounds check
}

char Lexer::advance() {
    char c = source_[current_++]; // consume current char
    if (c == '\n') {              // newline
        line_++;
        col_ = 1;
    }
    else {
        col_++; // advance column on same line
    }
    return c;
}

bool Lexer::match(char expected) {
    if (is_at_end() || source_[current_] != expected) { // no match
        return false; 
    }
    advance(); // consume matched char
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

bool Lexer::is_alpha(char c) const {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

bool Lexer::is_digit(char c) const {
    return std::isdigit(static_cast<unsigned char>(c)); // ASCII digits only
}

bool Lexer::is_alnum(char c) const {
    return is_alpha(c) || is_digit(c);
} // Combined
