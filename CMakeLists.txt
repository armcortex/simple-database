cmake_minimum_required(VERSION 3.22)
project(simple_database C)

set(CMAKE_C_STANDARD 11)

# Download Dependencies
Include(FetchContent)

# Download cJSON
FetchContent_Declare(
        cJSON
        GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
        GIT_TAG        v1.7.16
)
FetchContent_MakeAvailable(cJSON)

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
        src/database.c
        src/database.h
        src/parser.c
        src/parser.h
        src/rpn.c
        src/rpn.h
        src/test_fn_env.c
        src/test_fn_env.h
        src/operator.c
        src/operator.h
        src/table.h
)

add_library(simple_database_lib ${PROJ_SOURCES})

target_include_directories(simple_database_lib PUBLIC
        src
        ${cJSON_SOURCE_DIR}
        ${cJSON_BINARY_DIR}
)

link_directories(${cJSON_BINARY_DIR})

add_executable(simple_database src/main.c)

target_link_libraries(simple_database PRIVATE
        simple_database_lib
        cJSON
)
