#include <ostream>
#include <string>

#include "interpreter_printer.hpp"
#include "util/value_format.hpp"

namespace {

std::string stmt_label(const Stmt &stmt) {
    if (std::holds_alternative<ExprStmt>(stmt.node)) {
        return "ExprStmt";
    }
    if (std::holds_alternative<LetStmt>(stmt.node)) {
        const auto &node = std::get<LetStmt>(stmt.node);
        return "Let " + node.name.text;
    }
    if (std::holds_alternative<BlockStmt>(stmt.node)) {
        return "Block";
    }
    if (std::holds_alternative<IfStmt>(stmt.node)) {
        return "If";
    }
    if (std::holds_alternative<WhileStmt>(stmt.node)) {
        return "While";
    }
    if (std::holds_alternative<BreakStmt>(stmt.node)) {
        return "Break";
    }
    if (std::holds_alternative<ContinueStmt>(stmt.node)) {
        return "Continue";
    }
    if (std::holds_alternative<ReturnStmt>(stmt.node)) {
        return "Return";
    }
    if (std::holds_alternative<FnStmt>(stmt.node)) {
        const auto &node = std::get<FnStmt>(stmt.node);
        return "Fn " + node.name.text;
    }
    return "Stmt";
}

} // namespace

void install_trace_printer(Interpreter &interpreter, std::ostream &out) {
    // create trace hook printing function and pass to interpreter
    interpreter.set_trace_hook([&out](const Stmt &stmt, const interpreter_detail::Value *value) {
        out << "Trace: " << stmt_label(stmt);
        if (value) {
            out << " -> " << interpreter_detail::value_to_string(*value);
        }
        out << '\n';
    });
}
