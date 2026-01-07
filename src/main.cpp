#include <fstream>
#include <iostream>
#include <sstream>
#include <optional>
#include <string>

#include "lexer.hpp"
#include "token.hpp"

// Read entire file into string
static bool read_file(const std::string &path, std::string &out) {
    // Open file path
    std::ifstream in(path);
    if (!in) return false; // failed to open

    // copy into string
    std::ostringstream buf;
    buf << in.rdbuf(); // Stream file into buffer
    out = buf.str(); // copy to output
    return true;
}

int main(int argc, char **argv) {
    // Missing input path
    if (argc < 2) {
        std::cerr << "Usage: basic-interpreter <path>\n";
        return 1;
    }

    // read file to string
    std::string source;
    if (!read_file(argv[1], source)) {
        std::cerr << "Error: failed to open file\n";
        return 1;
    }

    Lexer lexer(std::move(source)); // Create lexer
    std::vector<Token> tokens = lexer.scan_tokens(); // Lex entire input

    // lexing failure
    if (!lexer.errors().empty()) {
        for (const std::string &err : lexer.errors()) {
            std::cerr << err << "\n"; // Print each lexing error
        }
        return 1;
    }

    // Debug print of token stream
    for (const Token &token : tokens) {
        std::cout << token_type_name(token.type) << " '" << token.lexeme << "'\n";
    }

    return 0; // Success
}
