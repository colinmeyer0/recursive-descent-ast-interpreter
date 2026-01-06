cmake_minimum_required(VERSION 3.16)
project(basic_interpreter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(basic-interpreter
  src/main.cpp
  src/lexer.cpp
)

target_include_directories(basic-interpreter PRIVATE include)
