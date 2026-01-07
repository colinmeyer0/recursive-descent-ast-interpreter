#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "token.hpp"

// Forward declarations
struct Expr;
struct Stmt;

// pointers to expressions
using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

// === EXPRESSIONS ===
struct LiteralExpr { // int/bool
    Literal value;
};

struct IdentifierExpr { // identifier
    std::string name;
};

struct GroupingExpr { // parentheses, brackets, etc.
    ExprPtr expression;
};

struct UnaryExpr { // '!x', '-x', etc.
    Token op;
    ExprPtr right;
};

struct BinaryExpr { // expression operator expression ('3 + 4', 'x < y', etc.)
    ExprPtr left;
    Token op;
    ExprPtr right;
};

struct AssignExpr { // assignment
    std::string name;
    ExprPtr value;
};

struct CallExpr { // function call
    ExprPtr callee;
    std::vector<ExprPtr> arguments;
};

// all variants of expressions
using ExprVariant = std::variant<
    LiteralExpr,
    IdentifierExpr,
    GroupingExpr,
    UnaryExpr,
    BinaryExpr,
    AssignExpr,
    CallExpr>;

// wrapper for expression variants (with span)
struct Expr {
    ExprVariant node;
    Span span;
};


// === STATEMENTS ===

struct ExprStmt { // store expression
    ExprPtr expression;
};

struct LetStmt { // assignment
    std::string name;
    ExprPtr initializer; // can be nullptr if uninitialized
};

struct BlockStmt { // block of code (contained in braces)
    std::vector<StmtPtr> statements;
};

struct IfStmt { // if statement (else is optional)
    ExprPtr condition;
    StmtPtr then_branch;
    StmtPtr else_branch; // can be nullptr
};

struct WhileStmt { // while statement
    ExprPtr condition;
    StmtPtr body;
};

struct BreakStmt {}; // break statement
struct ContinueStmt {}; // continue statement

struct ReturnStmt { // return statement (optional value)
    ExprPtr value; // can be nullptr
};

struct FnStmt { // function statement (body = pointer to block statement)
    std::string name;
    std::vector<std::string> params;
    std::vector<StmtPtr> body; // block statement
};

// all variants of statements
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


// wrapper for statement variants (with span)
struct Stmt {
    StmtVariant node;
    Span span;
};
