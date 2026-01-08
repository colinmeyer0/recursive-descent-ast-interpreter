#include "parser.hpp"

#include <initializer_list>
#include <utility>

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

std::vector<StmtPtr> Parser::parse() {
    std::vector<StmtPtr> statements;
    while (!is_at_end()) {
        StmtPtr decl = declaration();
        if (decl) {
            statements.push_back(std::move(decl));
        }
    }
    return statements;
}

const std::vector<std::string> &Parser::errors() const {
    return errors_;
}

StmtPtr Parser::declaration() {
    try {
        if (match({TokenType::FN})) return fn_declaration();
        if (match({TokenType::LET})) return let_declaration();
        return statement();
    } catch (const ParseError &) {
        synchronize();
        return nullptr;
    }
}

StmtPtr Parser::statement() {
    if (match({TokenType::IF})) return if_statement();
    if (match({TokenType::WHILE})) return while_statement();
    if (match({TokenType::BREAK})) return break_statement();
    if (match({TokenType::CONTINUE})) return continue_statement();
    if (match({TokenType::RETURN})) return return_statement();
    if (match({TokenType::LEFT_BRACE})) return block_statement(previous());
    return expression_statement();
}

StmtPtr Parser::let_declaration() {
    const Token &let_token = previous();
    const Token &name = consume(TokenType::IDENTIFIER, "Expect variable name after 'let'.");
    ExprPtr initializer;
    if (match({TokenType::EQUAL})) {
        initializer = expression();
    }
    const Token &semi = consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    Span span = span_from(let_token.span, semi.span);
    return make_stmt(LetStmt{TextInfo{name.lexeme, name.span}, std::move(initializer)}, span);
}

StmtPtr Parser::fn_declaration() {
    const Token &fn_token = previous();
    const Token &name = consume(TokenType::IDENTIFIER, "Expect function name after 'fn'.");
    consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");

    std::vector<TextInfo> params;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            const Token &param = consume(TokenType::IDENTIFIER, "Expect parameter name.");
            params.push_back(TextInfo{param.lexeme, param.span});
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");

    consume(TokenType::LEFT_BRACE, "Expect '{' before function body.");
    const Token &left_brace = previous();
    std::vector<StmtPtr> body;
    while (!check(TokenType::RIGHT_BRACE) && !is_at_end()) {
        StmtPtr decl = declaration();
        if (decl) body.push_back(std::move(decl));
    }
    const Token &right_brace = consume(TokenType::RIGHT_BRACE, "Expect '}' after function body.");

    Span span = span_from(fn_token.span, right_brace.span);
    return make_stmt(FnStmt{TextInfo{name.lexeme, name.span}, std::move(params), std::move(body)}, span);
}

StmtPtr Parser::if_statement() {
    const Token &if_token = previous();
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    ExprPtr condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");

    StmtPtr then_branch = statement();
    StmtPtr else_branch;
    Span end_span = then_branch ? then_branch->span : previous().span;
    if (match({TokenType::ELSE})) {
        else_branch = statement();
        if (else_branch) end_span = else_branch->span;
    }

    Span span = span_from(if_token.span, end_span);
    return make_stmt(IfStmt{std::move(condition), std::move(then_branch), std::move(else_branch)}, span);
}

StmtPtr Parser::while_statement() {
    const Token &while_token = previous();
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    ExprPtr condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after while condition.");

    StmtPtr body = statement();
    Span end_span = body ? body->span : previous().span;
    Span span = span_from(while_token.span, end_span);
    return make_stmt(WhileStmt{std::move(condition), std::move(body)}, span);
}

StmtPtr Parser::break_statement() {
    const Token &break_token = previous();
    const Token &semi = consume(TokenType::SEMICOLON, "Expect ';' after 'break'.");
    Span span = span_from(break_token.span, semi.span);
    return make_stmt(BreakStmt{}, span);
}

StmtPtr Parser::continue_statement() {
    const Token &continue_token = previous();
    const Token &semi = consume(TokenType::SEMICOLON, "Expect ';' after 'continue'.");
    Span span = span_from(continue_token.span, semi.span);
    return make_stmt(ContinueStmt{}, span);
}

StmtPtr Parser::return_statement() {
    const Token &return_token = previous();
    ExprPtr value;
    if (!check(TokenType::SEMICOLON)) {
        value = expression();
    }
    const Token &semi = consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    Span end_span = value ? value->span : semi.span;
    Span span = span_from(return_token.span, end_span);
    return make_stmt(ReturnStmt{std::move(value)}, span);
}

StmtPtr Parser::block_statement(const Token &left_brace) {
    std::vector<StmtPtr> statements;
    while (!check(TokenType::RIGHT_BRACE) && !is_at_end()) {
        StmtPtr decl = declaration();
        if (decl) statements.push_back(std::move(decl));
    }
    const Token &right_brace = consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    Span span = span_from(left_brace.span, right_brace.span);
    return make_stmt(BlockStmt{std::move(statements)}, span);
}

StmtPtr Parser::expression_statement() {
    ExprPtr expr = expression();
    const Token &semi = consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    Span span = span_from(expr->span, semi.span);
    return make_stmt(ExprStmt{std::move(expr)}, span);
}

ExprPtr Parser::expression() {
    return assignment();
}

ExprPtr Parser::assignment() {
    ExprPtr expr = logic_or();
    if (match({TokenType::EQUAL})) {
        const Token &equals = previous();
        ExprPtr value = assignment();
        if (auto *ident = std::get_if<VariableExpr>(&expr->node)) {
            Span span = span_from(expr->span, value->span);
            return make_expr(AssignExpr{ident->name, std::move(value)}, span);
        }
        error_at(equals, "Invalid assignment target.");
    }
    return expr;
}

ExprPtr Parser::logic_or() {
    ExprPtr expr = logic_and();
    while (match({TokenType::OR_OR})) {
        Token op = previous();
        ExprPtr right = logic_and();
        Span span = span_from(expr->span, right->span);
        expr = make_expr(BinaryExpr{std::move(expr), Op{op.type, op.span}, std::move(right)}, span);
    }
    return expr;
}

ExprPtr Parser::logic_and() {
    ExprPtr expr = equality();
    while (match({TokenType::AND_AND})) {
        Token op = previous();
        ExprPtr right = equality();
        Span span = span_from(expr->span, right->span);
        expr = make_expr(BinaryExpr{std::move(expr), Op{op.type, op.span}, std::move(right)}, span);
    }
    return expr;
}

ExprPtr Parser::equality() {
    ExprPtr expr = comparison();
    while (match({TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL})) {
        Token op = previous();
        ExprPtr right = comparison();
        Span span = span_from(expr->span, right->span);
        expr = make_expr(BinaryExpr{std::move(expr), Op{op.type, op.span}, std::move(right)}, span);
    }
    return expr;
}

ExprPtr Parser::comparison() {
    ExprPtr expr = term();
    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL})) {
        Token op = previous();
        ExprPtr right = term();
        Span span = span_from(expr->span, right->span);
        expr = make_expr(BinaryExpr{std::move(expr), Op{op.type, op.span}, std::move(right)}, span);
    }
    return expr;
}

ExprPtr Parser::term() {
    ExprPtr expr = factor();
    while (match({TokenType::PLUS, TokenType::MINUS})) {
        Token op = previous();
        ExprPtr right = factor();
        Span span = span_from(expr->span, right->span);
        expr = make_expr(BinaryExpr{std::move(expr), Op{op.type, op.span}, std::move(right)}, span);
    }
    return expr;
}

ExprPtr Parser::factor() {
    ExprPtr expr = unary();
    while (match({TokenType::STAR, TokenType::SLASH})) {
        Token op = previous();
        ExprPtr right = unary();
        Span span = span_from(expr->span, right->span);
        expr = make_expr(BinaryExpr{std::move(expr), Op{op.type, op.span}, std::move(right)}, span);
    }
    return expr;
}

ExprPtr Parser::unary() {
    if (match({TokenType::BANG, TokenType::MINUS})) {
        Token op = previous();
        ExprPtr right = unary();
        Span span = span_from(op.span, right->span);
        return make_expr(UnaryExpr{Op{op.type, op.span}, std::move(right)}, span);
    }
    return call();
}

ExprPtr Parser::call() {
    ExprPtr expr = primary();
    while (match({TokenType::LEFT_PAREN})) {
        expr = finish_call(std::move(expr));
    }
    return expr;
}

ExprPtr Parser::finish_call(ExprPtr callee) {
    std::vector<ExprPtr> arguments;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            arguments.push_back(expression());
        } while (match({TokenType::COMMA}));
    }
    const Token &paren = consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
    Span span = span_from(callee->span, paren.span);
    return make_expr(CallExpr{std::move(callee), std::move(arguments)}, span);
}

ExprPtr Parser::primary() {
    if (match({TokenType::NUMBER, TokenType::TRUE, TokenType::FALSE})) {
        Token lit = previous();
        return make_expr(LiteralExpr{lit.literal}, lit.span);
    }
    if (match({TokenType::IDENTIFIER})) {
        Token name = previous();
        return make_expr(VariableExpr{name.lexeme}, name.span);
    }
    if (match({TokenType::LEFT_PAREN})) {
        const Token &left = previous();
        ExprPtr expr = expression();
        const Token &right = consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        Span span = span_from(left.span, right.span);
        return make_expr(GroupingExpr{std::move(expr)}, span);
    }
    error_at(peek(), "Expect expression.");
    throw ParseError{};
}

bool Parser::is_at_end() const {
    return peek().type == TokenType::EOF_;
}

const Token &Parser::peek() const {
    return tokens_[current_];
}

const Token &Parser::previous() const {
    return tokens_[current_ - 1];
}

const Token &Parser::advance() {
    if (!is_at_end()) current_++;
    return previous();
}

bool Parser::check(TokenType type) const {
    if (is_at_end()) return false;
    return peek().type == type;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

const Token &Parser::consume(TokenType type, const std::string &message) {
    if (check(type)) return advance();
    error_at(peek(), message);
    throw ParseError{};
}

void Parser::synchronize() {
    advance();
    while (!is_at_end()) {
        if (previous().type == TokenType::SEMICOLON) return;
        switch (peek().type) {
        case TokenType::LET:
        case TokenType::IF:
        case TokenType::WHILE:
        case TokenType::BREAK:
        case TokenType::CONTINUE:
        case TokenType::RETURN:
        case TokenType::FN:
            return;
        default:
            break;
        }
        advance();
    }
}

void Parser::error_at(const Token &token, const std::string &message) {
    errors_.push_back("Line " + std::to_string(token.span.pos.line) + ", col " +
                      std::to_string(token.span.pos.col) + ": " + message);
}

Span Parser::span_from(const Span &start, const Span &end) const {
    Span span;
    span.start = start.start;
    span.end = end.end;
    span.pos = start.pos;
    return span;
}

ExprPtr Parser::make_expr(ExprVariant node, Span span) {
    auto expr = std::make_unique<Expr>();
    expr->node = std::move(node);
    expr->span = span;
    return expr;
}

StmtPtr Parser::make_stmt(StmtVariant node, Span span) {
    auto stmt = std::make_unique<Stmt>();
    stmt->node = std::move(node);
    stmt->span = span;
    return stmt;
}
