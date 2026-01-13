#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

#include "ast.hpp"

/// environment.hpp contents
namespace interpreter_detail {
struct Function;
struct BuiltinFunction;

using Value = std::variant<std::monostate, int, bool, std::shared_ptr<Function>, std::shared_ptr<BuiltinFunction>>;

struct Environment {
    /// create environment with optional enclosing scope
    explicit Environment(std::shared_ptr<Environment> enclosing_env = nullptr);

    /// define a new name in the current scope, returns false if already defined
    bool define(const std::string &name, Value value);
    /// assign to an existing name, searching outward, returns false if undefined
    bool assign(const std::string &name, Value value);
    /// lookup name value, searching outward, returns nullptr if undefined
    const Value *get(const std::string &name) const;
    /// true if name exists in the current scope only
    bool has_local(const std::string &name) const;

    /// map of identifiers and their respective values for a given scope
    std::unordered_map<std::string, Value> values;
    /// parent scope for function environment
    std::shared_ptr<Environment> enclosing;
};

/// ast declaration node and closure environment (at definition time)
struct Function {
    const FnStmt *declaration = nullptr;
    /// environment from which the function was defined
    std::shared_ptr<Environment> closure;
};
} // namespace interpreter_detail
