#pragma once

// Recursive descent parser for tokens -> AST

#include <initializer_list>
#include <string>
#include <vector>

#include "ast.hpp"
#include "token.hpp"

class Parser {
  public:
    explicit Parser(std::vector<Token> tokens);

    std::vector<StmtPtr> parse();
    const std::vector<std::string> &errors() const;

  private:
    struct ParseError {};

    std::vector<Token> tokens_;
    std::vector<std::string> errors_;
    std::size_t current_ = 0;

    // declarations/statements
    StmtPtr declaration();
    StmtPtr statement();
    StmtPtr let_declaration();
    StmtPtr fn_declaration();
    StmtPtr if_statement();
    StmtPtr while_statement();
    StmtPtr break_statement();
    StmtPtr continue_statement();
    StmtPtr return_statement();
    StmtPtr block_statement(const Token &left_brace);
    StmtPtr expression_statement();

    // expressions
    ExprPtr expression();
    ExprPtr assignment();
    ExprPtr logic_or();
    ExprPtr logic_and();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr call();
    ExprPtr finish_call(ExprPtr callee);
    ExprPtr primary();

    // token helpers
    bool is_at_end() const;
    const Token &peek() const;
    const Token &previous() const;
    const Token &advance();
    bool check(TokenType type) const;
    bool match(std::initializer_list<TokenType> types);
    const Token &consume(TokenType type, const std::string &message);
    void synchronize();

    // error + span helpers
    void error_at(const Token &token, const std::string &message);
    Span span_from(const Span &start, const Span &end) const;
    ExprPtr make_expr(ExprVariant node, Span span);
    StmtPtr make_stmt(StmtVariant node, Span span);
};
