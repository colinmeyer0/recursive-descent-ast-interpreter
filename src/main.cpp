#include <fstream>
#include <iostream>
#include <sstream>
#include <optional>

#include "lexer.hpp"
#include "token.hpp"

// Read entire file into string
static std::optional<std::string> read_file(const std::string &path) {
    // Open file path
    std::ifstream in(path);
    if (!in) { // failed to open
        std::cerr << "Error: failed to open file '" << path << "'\n";
        return std::nullopt;
    }

    // copy into string
    std::ostringstream out;
    out << in.rdbuf(); // Stream file into buffer
    return out.str();
}

int main(int argc, char **argv) {
    // Missing input path
    if (argc < 2) {
        std::cerr << "Usage: basic-interpreter <path>\n";
        return 1;
    }

    auto source = read_file(argv[1]); // Load source file
    if (!source) return 1;
    
    Lexer lexer(std::move(*source)); // Create lexer with owned source
    std::vector<Token> tokens = lexer.scan_tokens(); // Lex entire input

    if (!lexer.errors().empty()) {
        for (const auto &err : lexer.errors()) {
            std::cerr << err << "\n"; // Print each lexing error
        }
        return 1; // Non-zero on lexing failures
    }

    // Debug print of token stream
    for (const auto &token : tokens) {
        std::cout << token_type_name(token.type) << " '" << token.lexeme
                  << "'\n";
    }

    return 0; // Success
}
