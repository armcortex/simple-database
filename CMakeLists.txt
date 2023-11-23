cmake_minimum_required(VERSION 3.22)
project(simple_database C)

set(CMAKE_C_STANDARD 11)

#enable_testing()
add_subdirectory(tests)

set(PROJ_SOURCES
        src/prompt.c
        src/prompt.h
        src/helper_functions.c
        src/helper_functions.h
        src/inputs.c
        src/inputs.h
        src/db_config.h
        src/cmd_functions.c
        src/cmd_functions.h
)
add_library(simple_database_lib ${PROJ_SOURCES})
target_include_directories(simple_database_lib PUBLIC src)

add_executable(simple_database src/main.c)
target_link_libraries(simple_database PRIVATE simple_database_lib)
