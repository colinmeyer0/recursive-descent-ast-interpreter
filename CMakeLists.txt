cmake_minimum_required(VERSION 3.20)
project(basic_interpreter LANGUAGES CXX)
set(CMAKE_CXX_EXTENSIONS OFF)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# sanitizers
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(ENABLE_ASAN)
	add_compile_options(-g -O0 -fsanitize=address -fno-omit-frame-pointer)
	add_link_options(-fsanitize=address)
endif()

# library
add_library(interpreter
	src/builtins.cpp
	src/environment.cpp
	src/lexer.cpp
	src/interpreter.cpp
	src/parser.cpp
	src/util/file_io.cpp
	src/util/cli.cpp
)
target_include_directories(interpreter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# main executable
add_executable(interpreter_cli src/main.cpp)
target_link_libraries(interpreter_cli PRIVATE interpreter)

# test executables
add_executable(lexer_test tests/lexer/lexer_test.cpp)
target_link_libraries(lexer_test PRIVATE interpreter)

add_executable(parser_test
	tests/parser/parser_test.cpp
	tests/parser/ast_printer.cpp
)
target_link_libraries(parser_test PRIVATE interpreter)

add_executable(interpreter_test tests/interpreter/interpreter_test.cpp)
target_link_libraries(interpreter_test PRIVATE interpreter)
