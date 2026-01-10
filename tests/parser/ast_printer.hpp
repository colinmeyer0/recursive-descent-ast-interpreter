#pragma once

#include <iosfwd>
#include <vector>

#include "ast.hpp"

// print AST nodes as an indented tree
void print_program(const std::vector<StmtPtr> &statements, std::ostream &out);

namespace ast_printer_detail {
// statement/expression printing
void print_expr(const Expr &expr, std::ostream &out, int indent);        // dispatch expression node printing
void print_stmt(const Stmt &stmt, std::ostream &out, int indent);        // dispatch statement nodes to a labeled tree view
void print_block(const BlockStmt &block, std::ostream &out, int indent); // print block contents shared by block and function bodies
void print_expr_ptr(const ExprPtr &expr, std::ostream &out, int indent); // print ExprPtr with null guard
void print_stmt_ptr(const StmtPtr &stmt, std::ostream &out, int indent); // print StmtPtr with null guard

// print helpers
std::string literal_to_string(const Literal &literal);                   // convert literal type to a display string
void print_line(std::ostream &out, int indent, const std::string &text); // print single line with specified indent
void print_indent(std::ostream &out, int indent);                        // print specified indent
} // namespace ast_printer_detail
