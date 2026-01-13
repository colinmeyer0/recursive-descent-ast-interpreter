#pragma once

#include <memory>
#include <stdexcept>
#include <vector>

#include "ast.hpp"
#include "environment.hpp"
#include "token.hpp"

class Interpreter {
  public:
    /// construct interpreter with a global environment
    Interpreter();

    /// execute a list of statements and collect any runtime errors
    void interpret(const std::vector<StmtPtr> &statements);
    /// runtime error messages collected during interpretation
    const std::vector<std::string> &errors() const;

  private:
    // bring environment definitions into scope
    using Value = interpreter_detail::Value;
    using Environment = interpreter_detail::Environment;
    using Function = interpreter_detail::Function;
    using BuiltinFunction = interpreter_detail::BuiltinFunction;

    /// sentinal-like error reporting type
    struct RuntimeError : public std::runtime_error {
        Span span;
        /// construct runtime error class
        RuntimeError(Span error_span, const std::string &message);
    };

    // loop/function signals
    struct BreakSignal {};
    struct ContinueSignal {};
    struct ReturnSignal {
        Value value;
    };

    // attributes
    std::vector<std::string> errors_;
    std::shared_ptr<Environment> globals_;
    std::shared_ptr<Environment> environment_;
    int loop_depth_ = 0;
    int function_depth_ = 0;

    /// execute a statement node
    void execute(const Stmt &stmt);
    /// evaluate an expression node
    Value evaluate(const Expr &expr);

    /// execute statements in a new environment
    void execute_block(const std::vector<StmtPtr> &statements, std::shared_ptr<Environment> env);
    /// evaluate a call expression, including arity and argument handling
    Value evaluate_call(const CallExpr &expr, const Span &span);
    /// convert literal tokens to runtime values
    Value value_from_literal(const Literal &literal) const;

    /// validate bool values and report type errors
    bool expect_bool(const Value &value, const Span &span, const std::string &context);
    /// validate int values and report type errors
    int expect_number(const Value &value, const Span &span, const std::string &context);
    /// compare values for equality
    bool values_equal(const Value &left, const Value &right) const;
    /// type name for error messages
    const char *value_type_name(const Value &value) const;

    /// raise a runtime error with span context
    void runtime_error(const Span &span, const std::string &message);
    /// format runtime errors for CLI display
    std::string format_error(const Span &span, const std::string &message) const;
};
