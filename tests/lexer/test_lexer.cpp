#include <iostream>
#include <string>

#include "lexer.hpp"
#include "token.hpp"
#include "util/file_io.hpp"
#include "util/cli.hpp"

// Debug print of token stream
void print_token_stream(std::vector<Token> tokens) {
    for (const Token &token : tokens) {
        std::cout << token_type_name(token.type) << " '" << token.lexeme << "'\n";
    }
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

    // use lexer to create token vector
    Lexer lexer(std::move(source)); // Create lexer
    std::vector<Token> tokens = lexer.scan_tokens(); // Lex entire input
    if (check_lexer_errors(lexer)) return 1; // lexing failure

    print_token_stream(tokens); // debug

    return 0; // Success
}
