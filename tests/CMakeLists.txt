cmake_minimum_required(VERSION 3.22)
project(Tests CXX)

set(CMAKE_CXX_STANDARD 17)

# Download Dependencies
Include(FetchContent)

# Download Catch2 testing framework
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(Catch2)


add_executable(tests_run
        tests_helper_function.cpp
        tests_check_command.cpp
)

target_link_libraries(tests_run PRIVATE Catch2::Catch2WithMain simple_database_lib)

include(Catch)
catch_discover_tests(tests_run)