#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

#include "environment.hpp"

namespace interpreter_detail {

/// callable wrapper for builtin functions
struct BuiltinFunction {
    /// implementation signature used by builtin callables
    using Impl = std::function<Value(const std::vector<Value> &)>;

    /// construct with a fixed arity and a callable implementation
    BuiltinFunction(std::size_t arity, Impl impl) : arity_(arity), impl_(std::move(impl)) {}

    /// construct a variadic builtin that accepts any number of arguments
    static BuiltinFunction variadic(Impl impl) {
        return BuiltinFunction(std::nullopt, std::move(impl));
    }

    /// return the number of arguments this builtin expects
    std::size_t arity() const {
        return arity_.value_or(0);
    }

    /// true when this builtin accepts a variable number of arguments
    bool is_variadic() const {
        return !arity_.has_value();
    }

    /// call the builtin with evaluated arguments
    Value call(const std::vector<Value> &arguments) const {
        return impl_(arguments);
    }

  private:
    /// stored arity for validation at call
    std::optional<std::size_t> arity_;
    /// function body for the builtin
    Impl impl_;

    BuiltinFunction(std::optional<std::size_t> arity, Impl impl) : arity_(arity), impl_(std::move(impl)) {}
};

// function declarations

/// register builtin callables in the given global environment
void register_builtins(Environment &globals);

} // namespace interpreter_detail
