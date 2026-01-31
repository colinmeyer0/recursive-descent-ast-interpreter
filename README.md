# Recursive-Descent AST Interpreter (C++20)

A small tree-walk interpreter for a C-flavored language, written in C++. Its design is focused on the core interpreter pipeline, including **lexing, parsing, AST, and evaluation**, with separation of stages, explicit ownership, and consistent source-location error reporting.

## Overview

This project serves as a testpiece of a modern interpreter front-end and execution engine, emphasizing design patterns and abstractions commonly used in compiler, systems, and embedded-related software. The implementation prioritizes explicit memory control and efficient execution.

Its structure is purposefully designed to expose the internal mechanics of the program, with optional tracing that prints textual representations of control flow during lexing, parsing, and interpretation execution stages.

### Core Concepts Implemented

- **Low-level design** using RAII, smart pointers, and `std::variant` to handle ownership, lifetimes, and tagged runtime values without relying on garbage collection.
- A complete **language execution pipeline**, implemented from fundamental principles: lexical analysis, recursive-descent parsing with operator precedence, abstract syntax tree (AST) construction, and tree-walk evaluation.
- **Modular and testable architecture**, with clearly separated stages, stage-specific debug tooling, and span-specific error diagnostics tailored to accurate debugging.

## Architecture

### Lexer (Tokenization)

- Converts source text into a vector of tokens.
- Recognizes keywords (`let`, `if`, `while`, `fn`, etc.), identifiers, literals, operators, and punctuation, while rejecting whitespace.
- Tracks source spans (byte offsets and 1-based line/column) for each token.
- Produces consistent error reporting formatted as: `Line <n>, col <m>: <message>`.

### Recursive Descent Parser

- Consumes tokens and produces an AST. 
- AST nodes are represented with `std::variant` and `std::unique_ptr` to make lifetimes explicit and avoid ambiguous ownership.
- Error recovery uses synchronization signals (skip to next `;` or statement boundary) to continue parsing after failures.

### Tree-walk Interpreter

- Evaluates the AST directly.
- Lexical scoping implemented via an `Environment` chain, with local scope definition checks and outward lookup/assignment.
- Control-flow statements (`break`, `continue`, and `return`) implemented using internal signals to cleanly interrupt evaluation.
- Built-ins registered into global environment at interpreter construction.

## Language Features

### Types

- `number` (exclusively `int` under the current implementation)
- `boolean` (`true` / `false`)
- `nil` (used for “no value” / absence of return)

### Expressions

- Arithmetic: `+  -  *  /`
- Comparison: `<  <=  >  >=  ==  !=`
- Logical: `&&  ||  !` (with short-circuit evaluation)
- Grouping: `( ... )`
- Assignment: `x = expr`

### Statements

- Variable declaration (initialization required): `let x = expr;`
- Expression statement: `expr;`
- Block scope: `{ ... }`
- Conditionals:

  ```rust
  if (condition) { ... } else { ... }
  ```
- While loops + control flow:

  ```rust
  while (condition) { ... }
  break;
  continue;
  ```
- Functions:

  ```rust
  fn name(param1, param2) { ... }
  return expr;
  ```

### Built-ins

- `print(...)` — variadic output helper

### Comments

- Line comments: `// <text>`



## Example program

```rust
fn add(a, b) {
    return a + b;
}

fn main() {
    let x = 2;
    let y = 3;
    let z = add(x, y * 10);
    print(z);
}

main();
```

## Usage

Build with CMake and run the CLI against a source file:

```bash
cmake -S . -B build
cmake --build build
./build/interpreter_cli path/to/program.txt
```
- Stage-specific test executables: `./build/lexer_test`, `./build/parser_test`, `./build/interpreter_test`
- Example inputs: `tests/data/`


## Reflections and Challenges

This section captures my design process and shortcomings/tradeoffs made while stepping into a realm of programming that was foreign to me.

### Limitations / Issues Encountered

- **Higher-order functions / closures:** Attempted to allow first-class functions with captured environments, but failed to resolve memory leaks caused by expired closure due to pointer ownership complications. I am looking to safely implement this feature in the future.
- **Deep type nesting:** AST and runtime types are composed of many nested structs/variants, which makes some files hard to read.

### Potential Optimizations

- Cache resolved variable slots in the interpreter to avoid repeated environment lookups.
- Develop a bytecode VM to reduce recursive AST evaluation overhead.
