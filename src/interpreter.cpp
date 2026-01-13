#include <utility>

#include "builtins.hpp"
#include "interpreter.hpp"

Interpreter::RuntimeError::RuntimeError(Span error_span, const std::string &message) // define error construction
    : std::runtime_error(message), span(error_span) {}

Interpreter::Interpreter() // construct interpreter with Environment class as shared pointer
    : globals_(std::make_shared<Environment>()), environment_(globals_) {
    register_builtins(*globals_);
}

void Interpreter::interpret(const std::vector<StmtPtr> &statements) {
    // execute top-level statements, record first runtime error
    errors_.clear(); // clear errors in case interpret is called more than once
    try {
        for (const StmtPtr &stmt : statements) {
            if (stmt) { // ensure statement is valid
                execute(*stmt);
            }
        }
    } catch (const RuntimeError &err) { // catch errors
        errors_.push_back(format_error(err.span, err.what()));
    }
}

const std::vector<std::string> &Interpreter::errors() const {
    return errors_;
}

void Interpreter::execute(const Stmt &stmt) {
    // expression statement
    if (std::holds_alternative<ExprStmt>(stmt.node)) {
        const ExprStmt &node = std::get<ExprStmt>(stmt.node);
        if (node.expression) {
            evaluate(*node.expression);
        }
        return;
    }

    // variable declaration
    if (std::holds_alternative<LetStmt>(stmt.node)) {
        const LetStmt &node = std::get<LetStmt>(stmt.node);
        Value value = evaluate(*node.initializer);
        if (!environment_->define(node.name.text, std::move(value))) { // add to environment if variable hasn't already been declared
            runtime_error(node.name.span, "Variable already declared in this scope: '" + node.name.text + "'.");
        }
        return;
    }

    // block scope
    if (std::holds_alternative<BlockStmt>(stmt.node)) {
        const BlockStmt &node = std::get<BlockStmt>(stmt.node);
        std::shared_ptr<Environment> block_env = std::make_shared<Environment>(environment_);
        execute_block(node.statements, std::move(block_env));
        return;
    }

    // if/else branching
    if (std::holds_alternative<IfStmt>(stmt.node)) {
        const IfStmt &node = std::get<IfStmt>(stmt.node);
        bool condition = expect_bool(evaluate(*node.condition), node.condition->span, "if condition");
        if (condition) { // evaluate then branch
            if (node.then_branch) {
                execute(*node.then_branch);
            }
        }
        else if (node.else_branch) { // evaluate else brach
            execute(*node.else_branch);
        }
        return;
    }

    // while loop with break/continue support
    if (std::holds_alternative<WhileStmt>(stmt.node)) {
        const WhileStmt &node = std::get<WhileStmt>(stmt.node);
        ++loop_depth_;
        try {
            while (expect_bool(evaluate(*node.condition), node.condition->span, "while condition")) { // evaluate condition
                try {
                    if (node.body) {
                        execute(*node.body);
                    }
                } catch (const ContinueSignal &) {
                    continue;
                } catch (const BreakSignal &) {
                    break;
                }
            }
            // error in while body
        } catch (...) {
            --loop_depth_;
            throw;
        }
        // exit loop
        --loop_depth_;
        return;
    }

    // loop control statements
    if (std::holds_alternative<BreakStmt>(stmt.node)) {
        if (loop_depth_ == 0) {
            runtime_error(stmt.span, "Break used outside of a loop.");
        }
        throw BreakSignal{};
    }
    if (std::holds_alternative<ContinueStmt>(stmt.node)) {
        if (loop_depth_ == 0) {
            runtime_error(stmt.span, "Continue used outside of a loop.");
        }
        throw ContinueSignal{};
    }

    // return from function
    if (std::holds_alternative<ReturnStmt>(stmt.node)) {
        const ReturnStmt &node = std::get<ReturnStmt>(stmt.node);
        if (function_depth_ == 0) { // no where to return to
            runtime_error(stmt.span, "Return used outside of a function.");
        }
        Value value = node.value ? evaluate(*node.value) : Value{std::monostate{}};
        throw ReturnSignal{std::move(value)};
    }

    // function declaration closes over current environment
    if (std::holds_alternative<FnStmt>(stmt.node)) {
        const FnStmt &node = std::get<FnStmt>(stmt.node);
        std::shared_ptr<Function> fn = std::make_shared<Function>();
        fn->declaration = &node;
        fn->closure = environment_;
        if (!environment_->define(node.name.text, fn)) { // declare function if not declared already
            runtime_error(node.name.span, "Function already declared in this scope: '" + node.name.text + "'.");
        }
        return;
    }
}

Interpreter::Value Interpreter::evaluate(const Expr &expr) {
    // literal value
    if (std::holds_alternative<LiteralExpr>(expr.node)) {
        const LiteralExpr &node = std::get<LiteralExpr>(expr.node);
        return value_from_literal(node.value);
    }

    // variable/function reference
    if (std::holds_alternative<IdentifierExpr>(expr.node)) {
        const IdentifierExpr &node = std::get<IdentifierExpr>(expr.node);
        const Value *value = environment_->get(node.name.text);
        if (!value) {
            runtime_error(node.name.span, "Undefined identifier '" + node.name.text + "'.");
        }
        return *value;
    }

    // grouping just evaluates the inner expression
    if (std::holds_alternative<GroupingExpr>(expr.node)) {
        const GroupingExpr &node = std::get<GroupingExpr>(expr.node);
        return evaluate(*node.expression);
    }

    // unary operators
    if (std::holds_alternative<UnaryExpr>(expr.node)) {
        const UnaryExpr &node = std::get<UnaryExpr>(expr.node);
        Value right = evaluate(*node.right);
        switch (node.op.type) {
        case TokenType::MINUS:
            return -expect_number(right, node.op.span, "unary minus");
        case TokenType::BANG:
            return !expect_bool(right, node.op.span, "logical not");
        default:
            runtime_error(node.op.span, "Unsupported unary operator.");
        }
    }

    // binary operators with short-circuit logic
    if (std::holds_alternative<BinaryExpr>(expr.node)) {
        const BinaryExpr &node = std::get<BinaryExpr>(expr.node);

        // boolean values
        if (node.op.type == TokenType::AND_AND) {
            // evaluate left of binary expression
            bool left = expect_bool(evaluate(*node.left), node.left->span, "logical and");
            if (!left) { // short circuit
                return false;
            }
            // evaluate right of binary expression
            return expect_bool(evaluate(*node.right), node.right->span, "logical and");
        }

        if (node.op.type == TokenType::OR_OR) {
            bool left = expect_bool(evaluate(*node.left), node.left->span, "logical or");
            if (left) { // short circuit
                return true;
            }
            return expect_bool(evaluate(*node.right), node.right->span, "logical or");
        }

        // numerical values
        Value left = evaluate(*node.left);
        Value right = evaluate(*node.right);

        switch (node.op.type) {
        case TokenType::PLUS:
            return expect_number(left, node.op.span, "addition") +
                   expect_number(right, node.op.span, "addition");
        case TokenType::MINUS:
            return expect_number(left, node.op.span, "subtraction") -
                   expect_number(right, node.op.span, "subtraction");
        case TokenType::STAR:
            return expect_number(left, node.op.span, "multiplication") *
                   expect_number(right, node.op.span, "multiplication");
        case TokenType::SLASH: {
            int denominator = expect_number(right, node.op.span, "division");
            if (denominator == 0) {
                runtime_error(node.op.span, "Division by zero.");
            }
            return expect_number(left, node.op.span, "division") / denominator;
        }
        case TokenType::GREATER:
            return expect_number(left, node.op.span, "comparison") >
                   expect_number(right, node.op.span, "comparison");
        case TokenType::GREATER_EQUAL:
            return expect_number(left, node.op.span, "comparison") >=
                   expect_number(right, node.op.span, "comparison");
        case TokenType::LESS:
            return expect_number(left, node.op.span, "comparison") <
                   expect_number(right, node.op.span, "comparison");
        case TokenType::LESS_EQUAL:
            return expect_number(left, node.op.span, "comparison") <=
                   expect_number(right, node.op.span, "comparison");
        case TokenType::EQUAL_EQUAL:
            return values_equal(left, right);
        case TokenType::BANG_EQUAL:
            return !values_equal(left, right);
        default:
            runtime_error(node.op.span, "Unsupported binary operator.");
        }
    }

    // assignment expression
    if (std::holds_alternative<AssignExpr>(expr.node)) {
        const AssignExpr &node = std::get<AssignExpr>(expr.node);
        Value value = evaluate(*node.value);
        if (!environment_->assign(node.name.text, value)) { // assign value if variable is defined
            runtime_error(node.name.span, "Undefined variable '" + node.name.text + "'.");
        }
        return value;
    }

    // call expression
    if (std::holds_alternative<CallExpr>(expr.node)) {
        const CallExpr &node = std::get<CallExpr>(expr.node);
        return evaluate_call(node, expr.span);
    }

    return Value{std::monostate{}}; // expression doesn't return value
}

void Interpreter::execute_block(const std::vector<StmtPtr> &statements, std::shared_ptr<Environment> env) {
    // temporarily swap environments to execute block scope
    std::shared_ptr<Environment> previous = environment_;
    environment_ = std::move(env);
    try {
        for (const StmtPtr &stmt : statements) {
            if (stmt) {
                execute(*stmt);
            }
        }
    } catch (...) { // error -> abort to previous scope
        environment_ = previous;
        throw;
    }
    environment_ = previous; // return to previous scope
}

Interpreter::Value Interpreter::evaluate_call(const CallExpr &expr, const Span &span) {
    // only functions or builtins are callable
    Value callee = evaluate(*expr.callee);
    // handle builtin callables first
    if (std::holds_alternative<std::shared_ptr<BuiltinFunction>>(callee)) {
        const std::shared_ptr<BuiltinFunction> &builtin = std::get<std::shared_ptr<BuiltinFunction>>(callee);
        if (!builtin) {
            runtime_error(span, "Attempted to call an invalid builtin.");
        }

        // enforce builtin arity before evaluating arguments
        if (!builtin->is_variadic()) {
            const std::size_t expected = builtin->arity();
            if (expr.arguments.size() != expected) {
                runtime_error(expr.paren_span, "Expected " + std::to_string(expected) +
                                                   " arguments but got " + std::to_string(expr.arguments.size()) + ".");
            }
        }

        // evaluate arguments left-to-right for builtin call
        std::vector<Value> arguments;
        arguments.reserve(expr.arguments.size());
        for (const ExprPtr &arg : expr.arguments) {
            arguments.push_back(evaluate(*arg));
        }
        // call builtin implementation
        return builtin->call(arguments);
    }

    if (!std::holds_alternative<std::shared_ptr<Function>>(callee)) {
        runtime_error(span, "Can only call functions or builtins.");
    }

    // check environment reference to function
    const std::shared_ptr<Function> &function = std::get<std::shared_ptr<Function>>(callee);
    if (!function || !function->declaration) {
        runtime_error(span, "Attempted to call an invalid function.");
    }

    // enforce arity before evaluating arguments
    const std::vector<TextInfo> &params = function->declaration->params;
    if (expr.arguments.size() != params.size()) {
        runtime_error(expr.paren_span, "Expected " + std::to_string(params.size()) +
            " arguments but got " + std::to_string(expr.arguments.size()) + ".");
    }

    // evaluate argument expressions left-to-right
    std::vector<Value> arguments;
    arguments.reserve(expr.arguments.size());
    for (const ExprPtr &arg : expr.arguments) {
        arguments.push_back(evaluate(*arg));
    }

    // bind parameters in a new environment connected to the closure
    std::shared_ptr<Environment> call_env = std::make_shared<Environment>(function->closure);
    for (std::size_t i = 0; i < params.size(); ++i) {
        if (!call_env->define(params[i].text, arguments[i])) { // define parameters in new environment if not already defined
            runtime_error(params[i].span, "Duplicate parameter name '" + params[i].text + "'.");
        }
    }

    // execute function body and capture return signal
    ++function_depth_;
    try {
        execute_block(function->declaration->body.statements, std::move(call_env));
    } catch (const ReturnSignal &signal) { // return from function -> return value
        --function_depth_;
        return signal.value;
    } catch (...) { // unknown error in function body
        --function_depth_;
        throw; // rethrow current exception back up call stack
    }
    --function_depth_;
    return Value{std::monostate{}};
}

Interpreter::Value Interpreter::value_from_literal(const Literal &literal) const {
    // convert token literal to runtime value
    if (std::holds_alternative<std::monostate>(literal)) {
        return std::monostate{};
    }
    if (std::holds_alternative<int>(literal)) {
        return std::get<int>(literal);
    }
    else {
        return std::get<bool>(literal);
    }
}

bool Interpreter::expect_bool(const Value &value, const Span &span, const std::string &context) {
    // type check for boolean contexts
    if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value);
    }
    runtime_error(span, "Expected boolean in " + context + ", got " + std::string(value_type_name(value)) + ".");
    return false;
}

int Interpreter::expect_number(const Value &value, const Span &span, const std::string &context) {
    // type check for numeric contexts
    if (std::holds_alternative<int>(value)) {
        return std::get<int>(value);
    }
    runtime_error(span, "Expected number in " + context + ", got " + std::string(value_type_name(value)) + ".");
    return 0;
}

bool Interpreter::values_equal(const Value &left, const Value &right) const {
    // equality is strict by type; callables compare by identity
    if (left.index() != right.index()) { // not same type
        return false;
    }
    if (std::holds_alternative<std::monostate>(left)) {
        return true;
    }
    if (std::holds_alternative<int>(left)) {
        return std::get<int>(left) == std::get<int>(right);
    }
    if (std::holds_alternative<bool>(left)) {
        return std::get<bool>(left) == std::get<bool>(right);
    }
    if (std::holds_alternative<std::shared_ptr<Function>>(left)) {
        return std::get<std::shared_ptr<Function>>(left) == std::get<std::shared_ptr<Function>>(right);
    }
    return std::get<std::shared_ptr<BuiltinFunction>>(left) ==
           std::get<std::shared_ptr<BuiltinFunction>>(right);
}

const char *Interpreter::value_type_name(const Value &value) const {
    // map runtime value to human-readable name
    if (std::holds_alternative<std::monostate>(value)) {
        return "nil";
    }
    if (std::holds_alternative<int>(value)) {
        return "number";
    }
    if (std::holds_alternative<bool>(value)) {
        return "boolean";
    }
    if (std::holds_alternative<std::shared_ptr<Function>>(value)) {
        return "function";
    }
    return "builtin";
}

void Interpreter::runtime_error(const Span &span, const std::string &message) {
    // throw to unwind interpreter to the top-level
    throw RuntimeError(span, message);
}

std::string Interpreter::format_error(const Span &span, const std::string &message) const {
    // match lexer/parser error format
    return "Line " + std::to_string(span.pos.line) + ", col " + std::to_string(span.pos.col) + ": " + message;
}
