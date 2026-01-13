#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "token.hpp"

// forward declarations
struct Expr;
struct Stmt;

// pointers to expressions
using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

// === HELPER TYPES ===

/// raw text and span
struct TextInfo {
    std::string text;
    /// span of only the text
    Span span;
};

/// token type and span
struct Op {
    /// operator type
    TokenType type;
    /// operator span
    Span span;
};

// === EXPRESSIONS ===
// definition: produces a value

/// int or bool
struct LiteralExpr {
    Literal value;
};

/// identifier
struct IdentifierExpr {
    /// identifier text and span (same span as Expr wrapper in this case)
    TextInfo name;
};

/// parentheses, brackets, etc.
struct GroupingExpr {
    ExprPtr expression;
};

/// '!x', '-x', etc
struct UnaryExpr {
    Op op;
    ExprPtr right;
};

/// expression operator expression
struct BinaryExpr {
    ExprPtr left;
    Op op;
    ExprPtr right;
};

/// assignment
struct AssignExpr {
    TextInfo name;
    ExprPtr value;
};

/// function call
struct CallExpr {
    ExprPtr callee;
    std::vector<ExprPtr> arguments;
    /// span of parentheses for error reporting
    Span paren_span;
};

/// all variants of expressions
using ExprVariant = std::variant<
    LiteralExpr,
    IdentifierExpr,
    GroupingExpr,
    UnaryExpr,
    BinaryExpr,
    AssignExpr,
    CallExpr>;

/// wrapper for expression variants with span
struct Expr {
    ExprVariant node;
    Span span;
};

// === STATEMENTS ===
// definition: performs an action

/// store expression
struct ExprStmt {
    ExprPtr expression;
};

/// declaration (must be initialized)
struct LetStmt {
    TextInfo name;
    /// never nullptr
    ExprPtr initializer;
};

/// block of code (contained in braces)
struct BlockStmt {
    std::vector<StmtPtr> statements;
};

/// else is optional
struct IfStmt {
    ExprPtr condition;
    StmtPtr then_branch;
    /// can be nullptr
    StmtPtr else_branch;
};

struct WhileStmt {
    ExprPtr condition;
    StmtPtr body;
};

struct BreakStmt {};
struct ContinueStmt {};

/// optional value
struct ReturnStmt {
    ExprPtr value;
};

/// function statement
struct FnStmt {
    TextInfo name;
    std::vector<TextInfo> params;
    /// not a pointer
    BlockStmt body;
};

/// all variants of statements
using StmtVariant = std::variant<
    ExprStmt,
    LetStmt,
    BlockStmt,
    IfStmt,
    WhileStmt,
    BreakStmt,
    ContinueStmt,
    ReturnStmt,
    FnStmt>;

/// wrapper for statement variants (with span)
struct Stmt {
    StmtVariant node;
    Span span;
};
