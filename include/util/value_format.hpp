#pragma once

#include <string>

#include "environment.hpp"

namespace interpreter_detail {
/// convert Value to a string for builtin output
inline std::string value_to_string(const Value &value) {
    if (std::holds_alternative<std::monostate>(value)) {
        return "nil";
    }
    if (std::holds_alternative<int>(value)) {
        return std::to_string(std::get<int>(value));
    }
    if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value) ? "true" : "false";
    }
    if (std::holds_alternative<std::shared_ptr<Function>>(value)) {
        return "function";
    }
    return "builtin";
}

} // namespace interpreter_detail
