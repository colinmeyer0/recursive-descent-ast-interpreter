#include <util/cli.hpp>

bool check_lexer_errors(const Lexer& lexer) {
    // lexing failure
    if (!lexer.errors().empty()) {
        for (const auto &err : lexer.errors()) {
            std::cerr << err << "\n"; // Print each lexing error
        }
        return true;
    }
    return false; // no failures
}