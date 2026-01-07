cmake_minimum_required(VERSION 3.20)
project(basic_interpreter LANGUAGES CXX)
set(CMAKE_CXX_EXTENSIONS OFF)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# library
add_library(interpreter
	src/lexer.cpp
	src/util/file_io.cpp
	src/util/cli.cpp
)
target_include_directories(interpreter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# main executable
add_executable(interpreter_cli src/main.cpp)
target_link_libraries(interpreter_cli PRIVATE interpreter)

# test executables
add_executable(lexer_test tests/test_lexer.cpp)
target_link_libraries(lexer_test PRIVATE interpreter)