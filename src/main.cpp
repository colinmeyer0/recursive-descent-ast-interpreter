#include <iostream>
#include <string>

#include "lexer.hpp"
#include "parser.hpp"
#include "token.hpp"
#include "util/cli.hpp"
#include "util/file_io.hpp"

int main(int argc, char **argv) {
    // Missing input path
    if (argc < 2) {
        std::cerr << "Usage: ./build/interpreter_cli <path>\n";
        return 1;
    }

    // read file to string
    std::string source;
    if (!read_file(argv[1], source)) {
        std::cerr << "Error: failed to open file\n";
        return 1;
    }

    // use lexer to create token vector
    Lexer lexer(std::move(source));                  // Create lexer
    std::vector<Token> tokens = lexer.scan_tokens(); // Lex entire input
    if (check_lexer_errors(lexer)) return 1;         // lexing failure

    // use parser to create AST
    Parser parser(std::move(tokens));          // create parser
    std::vector<StmtPtr> ast = parser.parse(); // parse token array
    if (check_parser_errors(parser)) return 1; // parsing failure

    return 0; // Success
}
