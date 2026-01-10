#pragma once

// Recursive descent parser for tokens -> AST

#include <initializer_list>
#include <string>
#include <vector>

#include "ast.hpp"
#include "token.hpp"

class Parser {
  public:
    explicit Parser(std::vector<Token> tokens); // construct with vector of tokens

    std::vector<StmtPtr> parse();                   // main loop for parsing program; evaluates top level AST nodes and appends to array
    const std::vector<std::string> &errors() const; // returns array of parsing error messages

  private:
    struct ParseError {}; // erroneous state sentinal

    std::vector<Token> tokens_;       // input array of tokens
    std::vector<std::string> errors_; // array of parsing error messages
    std::size_t current_ = 0;         // index in token array

    // declarations/statements
    StmtPtr declaration();                            // top-level dispatcher for declarations/statements, catches errors and returns nullptr
    StmtPtr statement();                              // determine type of statement and call respective function to evaluate
    StmtPtr let_declaration();                        // parse declaration and return pointer to LetStmt
    StmtPtr fn_declaration();                         // parse function and return pointer to FnStmt
    StmtPtr if_statement();                           // evaluate if statement and else branch and return pointer to IfStmt
    StmtPtr while_statement();                        // evaluate while statement and body and return pointer to WhileStmt
    StmtPtr break_statement();                        // return pointer to BreakStmt
    StmtPtr continue_statement();                     // return pointer to ContinueStmt
    StmtPtr return_statement();                       // evaluate return value and return pointer to ReturnStmt
    StmtPtr block_statement(const Token &left_brace); // accumulate statements in block and return pointer to BlockStmt
    StmtPtr expression_statement();                   // parse expression to create ExprStmt containing ExprPtr

    // expressions
    ExprPtr expression();                // head of operator precedence chain, returns pointer to expression type
    ExprPtr assignment();                // if AssignExpr: determine RHS expression and return, else: return deeper expression
    ExprPtr logic_or();                  // if || BinaryExpr: determine RHS and LHS and return, else: return deeper expression
    ExprPtr logic_and();                 // if && BinaryExpr: determine RHS and LHS and return, else: return deeper expression
    ExprPtr equality();                  // if equality BinaryExpr: determine RHS and LHS and return, else: return deeper expression
    ExprPtr comparison();                // if comparison BinaryExpr: determine RHS and LHS and return, else: return deeper expression
    ExprPtr term();                      // if term BinaryExpr: determine RHS and LHS and return, else: return deeper expression
    ExprPtr factor();                    // if factor BinaryExpr: determine RHS and LHS and return, else: return deeper expression
    ExprPtr unary();                     // if UnaryExpr: determine RHS and return, else: return deeper expression
    ExprPtr call();                      // gets primary expression then evaluates call expression/s and returns CallExpr pointer
    ExprPtr finish_call(ExprPtr callee); // evaluates call arguments and returns CallExpr pointer
    ExprPtr primary();                   // evaluates and returns literal, identifier, or grouping expressions and throws error if invalid

    // token helpers
    bool is_at_end() const;                                           // true if current token = EOF
    const Token &peek() const;                                        // return current token in array
    const Token &previous() const;                                    // return previous token in array
    const Token &advance();                                           // advance to next token and return previous
    bool check(TokenType type) const;                                 // check if type = current type
    bool match(std::initializer_list<TokenType> types);               // check if tokens matches the current token, advance and return new
    const Token &consume(TokenType type, const std::string &message); // if match: advance and return, else: add error and throw
    void synchronize();                                               // after error: advance tokens until erroneous statement is finished

    // error + span helpers
    void add_error(const Token &token, const std::string &message); // format error and add to error vector
    Span span_from(const Span &start, const Span &end) const;       // construct span from two different spans
    ExprPtr make_expr(ExprVariant node, Span span);                 // allocate and initialize AST expression node
    StmtPtr make_stmt(StmtVariant node, Span span);                 // allocate and initialize AST statement node
};
