#pragma once

// Core token and source location types used across the lexer/parser.

#include <cstddef>
#include <string>
#include <variant>

// Token kinds recognized by the lexer.
enum class TokenType {
    // Grouping
    LEFT_PAREN,  // (
    RIGHT_PAREN, // )
    LEFT_BRACE,  // {
    RIGHT_BRACE, // }

    // Statement structure
    SEMICOLON, // ;
    COMMA,     // ,

    // Arithmetic
    PLUS,  // +
    MINUS, // -
    STAR,  // *
    SLASH, // /

    // Assignment
    EQUAL, // =

    // Comparison
    EQUAL_EQUAL,   // ==
    BANG_EQUAL,    // !=
    LESS,          // <
    LESS_EQUAL,    // <=
    GREATER,       // >
    GREATER_EQUAL, // >=
    BANG,          // !

    // Logical
    AND_AND, // &&
    OR_OR,   // ||

    // Literals
    IDENTIFIER, // variable/function names
    NUMBER,     // integer literal

    // Keywords
    LET,
    IF,
    ELSE,
    WHILE,
    BREAK,
    CONTINUE,
    RETURN,
    FN,

    // Booleans
    TRUE,
    FALSE,

    // End of file
    EOF_
};

// Literal payload for tokens that carry values.
using Literal = std::variant<std::monostate, int, bool>;

// 1-based line/column position at a specific point in source.
struct SourcePos {
    int line = 1; // Line number.
    int col = 1;  // Column number.
};

// Absolute byte span plus the line/column at the start.
struct Span {
    std::size_t start = 0; // Inclusive start offset.
    std::size_t end = 0;   // Exclusive end offset.
    SourcePos pos;         // Line/col for start.
};

// A single scanned token, including lexeme and parsed literal.
struct Token {
    TokenType type;     // Token kind.
    std::string lexeme; // Raw text slice.
    Literal literal;    // Parsed value if applicable.
    Span span;          // Location metadata.
};

// Helpful string name for debugging output.
inline const char *token_type_name(TokenType type) {
    switch (type) {
    case TokenType::LEFT_PAREN:
        return "LEFT_PAREN";
    case TokenType::RIGHT_PAREN:
        return "RIGHT_PAREN";
    case TokenType::LEFT_BRACE:
        return "LEFT_BRACE";
    case TokenType::RIGHT_BRACE:
        return "RIGHT_BRACE";
    case TokenType::SEMICOLON:
        return "SEMICOLON";
    case TokenType::COMMA:
        return "COMMA";
    case TokenType::PLUS:
        return "PLUS";
    case TokenType::MINUS:
        return "MINUS";
    case TokenType::STAR:
        return "STAR";
    case TokenType::SLASH:
        return "SLASH";
    case TokenType::EQUAL:
        return "EQUAL";
    case TokenType::EQUAL_EQUAL:
        return "EQUAL_EQUAL";
    case TokenType::BANG_EQUAL:
        return "BANG_EQUAL";
    case TokenType::LESS:
        return "LESS";
    case TokenType::LESS_EQUAL:
        return "LESS_EQUAL";
    case TokenType::GREATER:
        return "GREATER";
    case TokenType::GREATER_EQUAL:
        return "GREATER_EQUAL";
    case TokenType::BANG:
        return "BANG";
    case TokenType::AND_AND:
        return "AND_AND";
    case TokenType::OR_OR:
        return "OR_OR";
    case TokenType::IDENTIFIER:
        return "IDENTIFIER";
    case TokenType::NUMBER:
        return "NUMBER";
    case TokenType::LET:
        return "LET";
    case TokenType::IF:
        return "IF";
    case TokenType::ELSE:
        return "ELSE";
    case TokenType::WHILE:
        return "WHILE";
    case TokenType::BREAK:
        return "BREAK";
    case TokenType::CONTINUE:
        return "CONTINUE";
    case TokenType::RETURN:
        return "RETURN";
    case TokenType::FN:
        return "FN";
    case TokenType::TRUE:
        return "TRUE";
    case TokenType::FALSE:
        return "FALSE";
    case TokenType::EOF_:
        return "EOF";
    }
    return "UNKNOWN"; // Fallback for unexpected enum values.
}
