cmake_minimum_required(VERSION 4.2)
project(brainfuck C)

set(CMAKE_C_STANDARD 11)

add_executable(bf)
include_directories(include)
add_subdirectory(src)
