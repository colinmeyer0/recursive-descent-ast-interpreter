#include "builtins.hpp"

#include <iostream>
#include <string>

#include "util/value_format.hpp"

namespace interpreter_detail {
/// name-function pair used by builtin registry
struct BuiltinDefinition {
    const char *name;
    BuiltinFunction function;
};

/// constructs Impl type (function) and passes it to variadic builtin constructor, then returns builtin
BuiltinFunction make_print_builtin() {
    return BuiltinFunction::variadic([](const std::vector<Value> &arguments) -> Value {
        // print each argument separated by spaces
        for (std::size_t i = 0; i < arguments.size(); i++) {
            if (i > 0) {
                std::cout << ' ';
            }
            std::cout << value_to_string(arguments[i]);
        }
        std::cout << '\n';
        return std::monostate{};
    });
}

/// registry of all builtins to install into the global environment
const BuiltinDefinition *builtin_registry(std::size_t &count) {
    static const BuiltinDefinition registry[] = {
        {"print", make_print_builtin()},
    };
    count = sizeof(registry) / sizeof(registry[0]);
    return registry;
}

/// install builtins into the provided global environment
void register_builtins(Environment &globals) {
    std::size_t count = 0;
    const BuiltinDefinition *registry = builtin_registry(count);
    for (std::size_t i = 0; i < count; ++i) {
        globals.define(registry[i].name, std::make_shared<BuiltinFunction>(registry[i].function));
    }
}

} // namespace interpreter_detail
