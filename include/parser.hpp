#pragma once

#include <initializer_list>
#include <string>
#include <vector>

#include "ast.hpp"
#include "token.hpp"

class Parser {
  public:
    /// construct with vector of tokens
    explicit Parser(std::vector<Token> tokens);

    /// main loop for parsing program, evaluates top level ast nodes and appends to array
    std::vector<StmtPtr> parse();
    /// returns array of parsing error messages
    const std::vector<std::string> &errors() const;

  private:
    /// erroneous state sentinal
    struct ParseError {};

    /// input array of tokens
    std::vector<Token> tokens_;
    /// array of parsing error messages
    std::vector<std::string> errors_;
    /// index in token array
    std::size_t current_ = 0;

    // declarations and statements

    /// top-level dispatcher for declarations and statements, catches errors and returns nullptr
    StmtPtr declaration();
    /// determine type of statement and call respective function to evaluate
    StmtPtr statement();
    /// parse declaration and return pointer to LetStmt
    StmtPtr let_declaration();
    /// parse function and return pointer to FnStmt
    StmtPtr fn_declaration();
    /// evaluate if statement and else branch and return pointer to IfStmt
    StmtPtr if_statement();
    /// evaluate while statement and body and return pointer to WhileStmt
    StmtPtr while_statement();
    /// return pointer to BreakStmt
    StmtPtr break_statement();
    /// return pointer to ContinueStmt
    StmtPtr continue_statement();
    /// evaluate return value and return pointer to ReturnStmt
    StmtPtr return_statement();
    /// accumulate statements in block and return pointer to BlockStmt
    StmtPtr block_statement(const Token &left_brace);
    /// parse expression to create ExprStmt containing ExprPtr
    StmtPtr expression_statement();

    // expressions

    /// head of operator precedence chain, returns pointer to expression type
    ExprPtr expression();
    /// if AssignExpr, determine rhs expression and return, else return deeper expression
    ExprPtr assignment();
    /// if || BinaryExpr, determine rhs and lhs and return, else return deeper expression
    ExprPtr logic_or();
    /// if && BinaryExpr, determine rhs and lhs and return, else return deeper expression
    ExprPtr logic_and();
    /// if equality BinaryExpr, determine rhs and lhs and return, else return deeper expression
    ExprPtr equality();
    /// if comparison BinaryExpr, determine rhs and lhs and return, else return deeper expression
    ExprPtr comparison();
    /// if term BinaryExpr, determine rhs and lhs and return, else return deeper expression
    ExprPtr term();
    /// if factor BinaryExpr, determine rhs and lhs and return, else return deeper expression
    ExprPtr factor();
    /// if UnaryExpr, determine rhs and return, else return deeper expression
    ExprPtr unary();
    /// gets primary expression then evaluates call expression/s and returns CallExpr pointer
    ExprPtr call();
    /// evaluates call arguments and returns CallExpr pointer
    ExprPtr finish_call(ExprPtr callee);
    /// evaluates and returns literal, identifier, or grouping expressions and throws error if invalid
    ExprPtr primary();

    // token helpers

    /// true if current token = EOF
    bool is_at_end() const;
    /// return current token in array
    const Token &peek() const;
    /// return previous token in array
    const Token &previous() const;
    /// advance to next token and return previous
    const Token &advance();
    /// check if type = current type
    bool check(TokenType type) const;
    /// check if tokens matches the current token, advance and return new
    bool match(std::initializer_list<TokenType> types);
    /// if match, advance and return, else add error and throw
    const Token &consume(TokenType type, const std::string &message);
    /// after error, advance tokens until erroneous statement is finished
    void synchronize();

    // error and span helpers

    /// format error and add to error vector
    void add_error(const Token &token, const std::string &message);
    /// construct span from two different spans
    Span span_from(const Span &start, const Span &end) const;
    /// allocate and initialize ast expression node
    ExprPtr make_expr(ExprVariant node, Span span);
    /// allocate and initialize ast statement node
    StmtPtr make_stmt(StmtVariant node, Span span);
};
