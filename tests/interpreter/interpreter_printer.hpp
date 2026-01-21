#pragma once

#include <iosfwd>

#include "interpreter.hpp"

/// attach a trace printer to the interpreter
void install_trace_printer(Interpreter &interpreter, std::ostream &out);
