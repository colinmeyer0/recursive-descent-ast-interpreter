#pragma once

#include "interpreter.hpp"
#include "lexer.hpp"
#include "parser.hpp"

/// check if lexer fails and print errors. true if errors, false if none
bool check_lexer_errors(const Lexer &lexer);
/// check if parser fails and print errors. true if errors, false if none
bool check_parser_errors(const Parser &parser);
/// check runtime errors and print them
bool check_interpreter_errors(const Interpreter &interpreter);
