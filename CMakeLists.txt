cmake_minimum_required(VERSION 3.31)
project(elf LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_executable(elf main.c)
target_include_directories(elf PUBLIC ${CMAKE_SOURCE_DIR}/macros)