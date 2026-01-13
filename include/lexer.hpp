#pragma once

#include <cctype>
#include <string>
#include <vector>

#include "token.hpp"

class Lexer {
  public:
    /// construct lexer class
    explicit Lexer(std::string source);

    /// scan full input into tokens
    std::vector<Token> scan_tokens();
    /// returns array of lexer error messages
    const std::vector<std::string> &errors() const;

  private:
    // main logic

    /// scan a single token starting at current_
    void scan_token();
    /// token without literal
    void add_token(TokenType type);
    /// token with literal value
    void add_token(TokenType type, Literal literal);
    /// format error and add to error vector
    void add_error(const std::string &message, const SourcePos &pos);

    // scanning functions

    /// discard whitespace
    bool is_whitespace(char c) const;
    /// scan single char tokens and add to token vector
    bool scan_single_char_token(char c);
    /// scan two char logical operators and add to token vector
    bool scan_two_char_operator(char c);
    /// scan '&&' or '||' and add to token vector
    bool scan_paired_operator(char c);
    /// scan '/' and add to token vector or discard comments '//'
    bool scan_slash_or_comment(char c);

    /// scan number and add token and literal value to token vector
    void scan_number();
    /// scan indentifier or keyword and add to token vector
    void scan_identifier_or_keyword();

    // helper functions

    /// true if current_ reached source end
    bool is_at_end() const;
    /// consume and return current char
    char advance();
    /// conditional consume if next char matches
    bool match(char expected);
    /// current char without consuming
    char peek() const;
    /// one-char lookahead without consuming
    char peek_next() const;
    /// identifier start
    bool is_alpha(char c) const;
    /// ascii digit
    bool is_digit(char c) const;
    /// identifier continuation
    bool is_alnum(char c) const;

    // input and output variables

    /// full source buffer
    std::string source_;
    /// output token list
    std::vector<Token> tokens_;
    /// collected error messages
    std::vector<std::string> errors_;

    // local variables

    /// start index of current lexeme
    std::size_t start_ = 0;
    /// cursor into source_
    std::size_t current_ = 0;
    /// 1-based line counter
    int line_ = 1;
    /// 1-based column counter
    int col_ = 1;
    /// line and col at start_
    SourcePos start_pos_;
};
