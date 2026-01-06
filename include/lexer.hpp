#pragma once

// Lexer turns a source string into a flat token stream.

#include <string>
#include <vector>

#include "token.hpp"

class Lexer {
  public:
    explicit Lexer(std::string source); // Owns source buffer.

    // Scan the full input into tokens (always appends EOF_).
    std::vector<Token> scan_tokens();
    // Human-friendly error strings for basic lexing errors.
    const std::vector<std::string> &errors() const;

  private:
    bool is_at_end() const;    // True if current_ reached source end.
    char advance();            // Consume and return current char.
    bool match(char expected); // Conditional consume if next char matches.
    char peek() const;         // Current char without consuming.
    char peek_next() const;    // One-char lookahead without consuming.

    void scan_token();              // Scan a single token starting at current_.
    void add_token(TokenType type); // Token without literal.
    void add_token(TokenType type,
                   Literal literal); // Token with literal value.
    void add_error(const std::string &message,
                   const SourcePos &pos); // Format error.

    bool is_alpha(char c) const; // Identifier start.
    bool is_digit(char c) const; // ASCII digit.
    bool is_alnum(char c) const; // Identifier continuation.

    std::string source_;              // Full source buffer.
    std::vector<Token> tokens_;       // Output token list.
    std::vector<std::string> errors_; // Collected error messages.

    std::size_t start_ = 0;   // Start index of current lexeme.
    std::size_t current_ = 0; // Cursor into source_.
    int line_ = 1;            // 1-based line counter.
    int col_ = 1;             // 1-based column counter.
    SourcePos start_pos_;     // Line/col at start_.
};
