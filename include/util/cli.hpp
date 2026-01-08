#include <iostream>

#include <lexer.hpp>
#include <parser.hpp>

bool check_lexer_errors(const Lexer& lexer); // check if lexer fails and print errors. true if errors, false if none
bool check_parser_errors(const Parser& parser); // check if parser fails and print errors. true if errors, false if none
