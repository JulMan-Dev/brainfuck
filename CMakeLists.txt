cmake_minimum_required(VERSION 4.2)
project(brainfuck C)

set(CMAKE_C_STANDARD 11)

add_executable(bf)
add_executable(bfc)
add_library(bf_rt_static STATIC)
add_library(bf_rt_shared SHARED)
add_library(bf_int_static STATIC)
add_library(bf_int_shared SHARED)
include_directories(include)
add_subdirectory(src)
