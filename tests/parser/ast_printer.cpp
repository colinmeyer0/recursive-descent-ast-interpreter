#include <ostream>
#include <string>

#include "ast_printer.hpp"
#include "token.hpp"

void print_program(const std::vector<StmtPtr> &statements, std::ostream &out) {
    ast_printer_detail::print_line(out, 0, "AST Textual Form:\n"); // print header

    // no statements
    if (statements.empty()) {
        ast_printer_detail::print_line(out, 0, "<empty>");
        return;
    }

    // print statements
    for (const auto &stmt : statements) {
        ast_printer_detail::print_stmt_ptr(stmt, out, 0);
    }
}

namespace ast_printer_detail {

void print_expr(const Expr &expr, std::ostream &out, int indent) {
    // literal expression
    if (std::holds_alternative<LiteralExpr>(expr.node)) {
        const auto &node = std::get<LiteralExpr>(expr.node);
        print_line(out, indent, "Literal: " + literal_to_string(node.value));
    }
    // identifier expression
    else if (std::holds_alternative<IdentifierExpr>(expr.node)) {
        const auto &node = std::get<IdentifierExpr>(expr.node);
        print_line(out, indent, "Identifier: " + node.name.text);
    }
    // grouping expression
    else if (std::holds_alternative<GroupingExpr>(expr.node)) {
        const auto &node = std::get<GroupingExpr>(expr.node);
        print_line(out, indent, "Grouping");
        print_expr_ptr(node.expression, out, indent + 1);
    }
    // unary expression
    else if (std::holds_alternative<UnaryExpr>(expr.node)) {
        const auto &node = std::get<UnaryExpr>(expr.node);
        print_line(out, indent, std::string("Unary: ") + token_type_name(node.op.type));
        print_expr_ptr(node.right, out, indent + 1);
    }
    // binary expression
    else if (std::holds_alternative<BinaryExpr>(expr.node)) {
        const auto &node = std::get<BinaryExpr>(expr.node);
        print_line(out, indent, std::string("Binary: ") + token_type_name(node.op.type));
        print_line(out, indent + 1, "Left");
        print_expr_ptr(node.left, out, indent + 2);
        print_line(out, indent + 1, "Right");
        print_expr_ptr(node.right, out, indent + 2);
    }
    // assignment expression
    else if (std::holds_alternative<AssignExpr>(expr.node)) {
        const auto &node = std::get<AssignExpr>(expr.node);
        print_line(out, indent, "Assign: " + node.name.text);
        print_expr_ptr(node.value, out, indent + 1);
    }
    // call expression
    else if (std::holds_alternative<CallExpr>(expr.node)) {
        const auto &node = std::get<CallExpr>(expr.node);
        print_line(out, indent, "Call");
        print_line(out, indent + 1, "Callee");
        print_expr_ptr(node.callee, out, indent + 2);
        if (node.arguments.empty()) {
            print_line(out, indent + 1, "Arguments: <none>");
        }
        else {
            print_line(out, indent + 1, "Arguments");
            for (const auto &arg : node.arguments) {
                print_expr_ptr(arg, out, indent + 2);
            }
        }
    }
}

void print_stmt(const Stmt &stmt, std::ostream &out, int indent) {
    // expression
    if (std::holds_alternative<ExprStmt>(stmt.node)) {
        const auto &node = std::get<ExprStmt>(stmt.node);
        print_line(out, indent, "ExprStmt");
        print_expr_ptr(node.expression, out, indent + 1);
    }
    // let statement
    else if (std::holds_alternative<LetStmt>(stmt.node)) {
        const auto &node = std::get<LetStmt>(stmt.node);
        print_line(out, indent, "Let: " + node.name.text);
        print_line(out, indent + 1, "Initializer");
        print_expr_ptr(node.initializer, out, indent + 2);
    }
    // block statement
    else if (std::holds_alternative<BlockStmt>(stmt.node)) {
        const auto &node = std::get<BlockStmt>(stmt.node);
        print_block(node, out, indent);
    }
    // if statement
    else if (std::holds_alternative<IfStmt>(stmt.node)) {
        const auto &node = std::get<IfStmt>(stmt.node);
        print_line(out, indent, "If");
        print_line(out, indent + 1, "Condition");
        print_expr_ptr(node.condition, out, indent + 2);
        print_line(out, indent + 1, "Then");
        print_stmt_ptr(node.then_branch, out, indent + 2);
        if (node.else_branch) {
            print_line(out, indent + 1, "Else");
            print_stmt_ptr(node.else_branch, out, indent + 2);
        }
    }
    // while statement
    else if (std::holds_alternative<WhileStmt>(stmt.node)) {
        const auto &node = std::get<WhileStmt>(stmt.node);
        print_line(out, indent, "While");
        print_line(out, indent + 1, "Condition");
        print_expr_ptr(node.condition, out, indent + 2);
        print_line(out, indent + 1, "Body");
        print_stmt_ptr(node.body, out, indent + 2);
    }
    // break statement
    else if (std::holds_alternative<BreakStmt>(stmt.node)) {
        print_line(out, indent, "Break");
    }
    // continue statement
    else if (std::holds_alternative<ContinueStmt>(stmt.node)) {
        print_line(out, indent, "Continue");
    }
    // return statement
    else if (std::holds_alternative<ReturnStmt>(stmt.node)) {
        const auto &node = std::get<ReturnStmt>(stmt.node);
        print_line(out, indent, "Return");
        if (node.value) {
            print_expr_ptr(node.value, out, indent + 1);
        }
        else {
            print_line(out, indent + 1, "<void>");
        }
    }
    // function statement
    else if (std::holds_alternative<FnStmt>(stmt.node)) {
        const auto &node = std::get<FnStmt>(stmt.node);
        print_line(out, indent, "Fn: " + node.name.text);
        if (node.params.empty()) {
            print_line(out, indent + 1, "Params: <none>");
        }
        else {
            print_line(out, indent + 1, "Params");
            for (const auto &param : node.params) {
                print_line(out, indent + 2, param.text);
            }
        }
        print_line(out, indent + 1, "Body");
        print_block(node.body, out, indent + 2);
    }
}

void print_block(const BlockStmt &block, std::ostream &out, int indent) {
    print_line(out, indent, "Block");
    if (block.statements.empty()) {
        print_line(out, indent + 1, "<empty>");
        return;
    }
    for (const auto &child : block.statements) {
        print_stmt_ptr(child, out, indent + 1);
    }
}

void print_expr_ptr(const ExprPtr &expr, std::ostream &out, int indent) {
    // null expression
    if (!expr) {
        print_line(out, indent, "<null-expr>");
        return;
    }

    // valid expression
    print_expr(*expr, out, indent);
}

void print_stmt_ptr(const StmtPtr &stmt, std::ostream &out, int indent) {
    // null statement
    if (!stmt) {
        print_line(out, indent, "<null-stmt>");
        return;
    }

    // valid statement
    print_stmt(*stmt, out, indent);
}

std::string literal_to_string(const Literal &literal) {
    if (std::holds_alternative<std::monostate>(literal)) {
        return "nil";
    }
    if (std::holds_alternative<bool>(literal)) {
        return std::get<bool>(literal) ? "true" : "false";
    }
    return std::to_string(std::get<int>(literal));
}

void print_line(std::ostream &out, int indent, const std::string &text) {
    print_indent(out, indent); // print specified indent
    out << text << '\n';       // print line
}

void print_indent(std::ostream &out, int indent) {
    for (int i = 0; i < indent; ++i) {
        out << "  ";
    }
}

} // namespace ast_printer_detail
