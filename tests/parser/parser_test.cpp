#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

#include "ast.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "util/cli.hpp"
#include "util/file_io.hpp"

std::string indent(int level) {
    return std::string(static_cast<std::size_t>(level * 2), ' ');
}

std::string literal_to_string(const Literal &literal) {
    if (std::holds_alternative<int>(literal)) {
        return std::to_string(std::get<int>(literal));
    }
    if (std::holds_alternative<bool>(literal)) {
        return std::get<bool>(literal) ? "true" : "false";
    }
    return "nil";
}

void print_expr(const Expr &expr, int level);

void print_stmt(const Stmt &stmt, int level) {
    std::visit(
        [&](const auto &node) {
            using T = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<T, ExprStmt>) {
                std::cout << indent(level) << "ExprStmt\n";
                print_expr(*node.expression, level + 1);
            } else if constexpr (std::is_same_v<T, LetStmt>) {
                std::cout << indent(level) << "Let " << node.name << "\n";
                if (node.initializer) {
                    print_expr(*node.initializer, level + 1);
                }
            } else if constexpr (std::is_same_v<T, BlockStmt>) {
                std::cout << indent(level) << "Block\n";
                for (const auto &child : node.statements) {
                    print_stmt(*child, level + 1);
                }
            } else if constexpr (std::is_same_v<T, IfStmt>) {
                std::cout << indent(level) << "If\n";
                std::cout << indent(level + 1) << "Condition\n";
                print_expr(*node.condition, level + 2);
                std::cout << indent(level + 1) << "Then\n";
                print_stmt(*node.then_branch, level + 2);
                if (node.else_branch) {
                    std::cout << indent(level + 1) << "Else\n";
                    print_stmt(*node.else_branch, level + 2);
                }
            } else if constexpr (std::is_same_v<T, WhileStmt>) {
                std::cout << indent(level) << "While\n";
                std::cout << indent(level + 1) << "Condition\n";
                print_expr(*node.condition, level + 2);
                std::cout << indent(level + 1) << "Body\n";
                print_stmt(*node.body, level + 2);
            } else if constexpr (std::is_same_v<T, BreakStmt>) {
                std::cout << indent(level) << "Break\n";
            } else if constexpr (std::is_same_v<T, ContinueStmt>) {
                std::cout << indent(level) << "Continue\n";
            } else if constexpr (std::is_same_v<T, ReturnStmt>) {
                std::cout << indent(level) << "Return\n";
                if (node.value) {
                    print_expr(*node.value, level + 1);
                }
            } else if constexpr (std::is_same_v<T, FnStmt>) {
                std::cout << indent(level) << "Fn " << node.name;
                if (!node.params.empty()) {
                    std::cout << " (";
                    for (std::size_t i = 0; i < node.params.size(); ++i) {
                        if (i > 0) std::cout << ", ";
                        std::cout << node.params[i];
                    }
                    std::cout << ")";
                }
                std::cout << "\n";
                for (const auto &child : node.body) {
                    print_stmt(*child, level + 1);
                }
            }
        },
        stmt.node);
}

void print_expr(const Expr &expr, int level) {
    std::visit(
        [&](const auto &node) {
            using T = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<T, LiteralExpr>) {
                std::cout << indent(level) << "Literal " << literal_to_string(node.value) << "\n";
            } else if constexpr (std::is_same_v<T, VariableExpr>) {
                std::cout << indent(level) << "Identifier " << node.name << "\n";
            } else if constexpr (std::is_same_v<T, GroupingExpr>) {
                std::cout << indent(level) << "Grouping\n";
                print_expr(*node.expression, level + 1);
            } else if constexpr (std::is_same_v<T, UnaryExpr>) {
                std::cout << indent(level) << "Unary " << node.op.lexeme << "\n";
                print_expr(*node.right, level + 1);
            } else if constexpr (std::is_same_v<T, BinaryExpr>) {
                std::cout << indent(level) << "Binary " << node.op.lexeme << "\n";
                print_expr(*node.left, level + 1);
                print_expr(*node.right, level + 1);
            } else if constexpr (std::is_same_v<T, AssignExpr>) {
                std::cout << indent(level) << "Assign " << node.name << "\n";
                print_expr(*node.value, level + 1);
            } else if constexpr (std::is_same_v<T, CallExpr>) {
                std::cout << indent(level) << "Call\n";
                std::cout << indent(level + 1) << "Callee\n";
                print_expr(*node.callee, level + 2);
                if (!node.arguments.empty()) {
                    std::cout << indent(level + 1) << "Args\n";
                    for (const auto &arg : node.arguments) {
                        print_expr(*arg, level + 2);
                    }
                }
            }
        },
        expr.node);
}

int main(int argc, char **argv) {
    // Missing input path
    if (argc < 2) {
        std::cerr << "Usage: basic-interpreter <path>\n";
        return 1;
    }

    // read file to string
    std::string source;
    if (!read_file(argv[1], source)) {
        std::cerr << "Error: failed to open file\n";
        return 1;
    }

    // use lexer to create token vector
    Lexer lexer(std::move(source)); // Create lexer
    std::vector<Token> tokens = lexer.scan_tokens(); // Lex entire input
    if (check_lexer_errors(lexer)) return 1; // lexing failure

    // use parser to create AST
    Parser parser(std::move(tokens)); // create parser
    std::vector<StmtPtr> program = parser.parse(); // parse token stream
    if (check_parser_errors(parser)) return 1; // parsing failure

    std::cout << "Program\n";
    for (const auto &stmt : program) {
        print_stmt(*stmt, 1);
    }
    return 0;
}
