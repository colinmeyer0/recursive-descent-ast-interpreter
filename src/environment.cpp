#include <utility>

#include "environment.hpp"

namespace interpreter_detail {

Environment::Environment(std::shared_ptr<Environment> enclosing_env) : enclosing(std::move(enclosing_env)) {}

bool Environment::define(const std::string &name, Value value) {
    // check if variable has already been declared
    if (has_local(name)) {
        return false;
    }
    values[name] = std::move(value); // add to map of definitions
    return true;
}

bool Environment::assign(const std::string &name, Value value) {
    // assign to local scope if present, otherwise check enclosing scopes
    std::unordered_map<std::string, Value>::iterator it = values.find(name);
    if (it != values.end()) {
        it->second = std::move(value);
        return true;
    }
    if (enclosing) { // recurse on enclosing scopes of current environment
        return enclosing->assign(name, std::move(value));
    }
    return false;
}

const Value *Environment::get(const std::string &name) const {
    // lookup variable value
    std::unordered_map<std::string, Value>::const_iterator it = values.find(name);
    if (it != values.end()) {
        return &it->second; // return value
    }
    if (enclosing) { // recurse on enclosing scopes
        return enclosing->get(name);
    }
    return nullptr; // value not found
}

bool Environment::has_local(const std::string &name) const {
    return values.find(name) != values.end(); // check local scope
}
} // namespace interpreter_detail
