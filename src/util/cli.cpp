#include <iostream>

#include "util/cli.hpp"

bool check_lexer_errors(const Lexer &lexer) {
    // lexing failure
    if (!lexer.errors().empty()) {
        for (const auto &err : lexer.errors()) {
            std::cerr << err << "\n"; // Print each lexing error
        }
        return true;
    }
    return false; // no failures
}

bool check_parser_errors(const Parser &parser) {
    if (!parser.errors().empty()) {
        for (const auto &err : parser.errors()) {
            std::cerr << err << "\n";
        }
        return true;
    }
    return false;
}

bool check_interpreter_errors(const Interpreter &interpreter) {
    if (!interpreter.errors().empty()) {
        for (const auto &err : interpreter.errors()) {
            std::cerr << err << "\n";
        }
        return true;
    }
    return false;
}
