#pragma once

// Lexer turns a source string into a vector of tokens

#include <cctype>
#include <string>
#include <unordered_map>
#include <vector>

#include "token.hpp"

class Lexer {
  public:
    explicit Lexer(std::string source); // construct Lexer class

    std::vector<Token> scan_tokens();               // Scan full input into tokens
    const std::vector<std::string> &errors() const; // returns array of lexer error messages

  private:
    // main logic
    void scan_token();                                                // Scan a single token starting at current_
    void add_token(TokenType type);                                   // Token without literal
    void add_token(TokenType type, Literal literal);                  // Token with literal value
    void add_error(const std::string &message, const SourcePos &pos); // format error and add to error vector

    // scanning functions
    bool is_whitespace(char c) const;    // discard whitespace
    bool scan_single_char_token(char c); // scan single char tokens and add to token vector
    bool scan_two_char_operator(char c); // scan two char logical operators and add to token vector
    bool scan_paired_operator(char c);   // scan '&&' or '||' and add to token vector
    bool scan_slash_or_comment(char c);  // scan '/' and add to token vector or discard comments '//'

    void scan_number();                // scan number and add token and literal value to token vector
    void scan_identifier_or_keyword(); // scan indentifier or keyword and add to token vector

    // helper functions
    bool is_at_end() const;      // True if current_ reached source end
    char advance();              // Consume and return current char
    bool match(char expected);   // Conditional consume if next char matches
    char peek() const;           // Current char without consuming
    char peek_next() const;      // One-char lookahead without consuming
    bool is_alpha(char c) const; // Identifier start
    bool is_digit(char c) const; // ASCII digit
    bool is_alnum(char c) const; // Identifier continuation

    // input and output variables
    std::string source_;              // Full source buffer
    std::vector<Token> tokens_;       // Output token list
    std::vector<std::string> errors_; // Collected error messages

    // local variables
    std::size_t start_ = 0;   // Start index of current lexeme
    std::size_t current_ = 0; // Cursor into source_
    int line_ = 1;            // 1-based line counter
    int col_ = 1;             // 1-based column counter
    SourcePos start_pos_;     // Line/col at start_
};
