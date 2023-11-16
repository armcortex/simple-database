cmake_minimum_required(VERSION 3.22)
project(simple_database C)

set(CMAKE_C_STANDARD 11)

add_executable(simple_database src/main.c
        src/main.h
        src/prompt.c
        src/prompt.h)
