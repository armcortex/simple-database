//
// Created by MCS51_M2 on 2023/11/17.
//

#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include <cstddef>
#include "../src/helper_functions.h"


TEST_CASE("Split and Free Functions", "[split]") {
    Splitter_t splitter = split_construct();

    SECTION("Split a simple string") {
        const char* str = "Hello,World,C,Programming";
        size_t num_tokens;
        char** tokens = splitter.run(str, ",", &num_tokens);

        REQUIRE(num_tokens == 4);
        REQUIRE(std::strncmp(tokens[0], "Hello", 5) == 0);
        REQUIRE(std::strncmp(tokens[1], "World", 5) == 0);
        REQUIRE(std::strncmp(tokens[2], "C", 1) == 0);
        REQUIRE(std::strncmp(tokens[3], "Programming", 11) == 0);

        splitter.free(tokens, num_tokens);
    }

    SECTION("Split an empty string") {
        const char* str = "";
        size_t num_tokens;
        char** tokens = splitter.run(str, ",", &num_tokens);

        REQUIRE(num_tokens == 0);

        splitter.free(tokens, num_tokens);
    }

    SECTION("Split a large string") {
        // Prepare large string data
        size_t max_cnt = 10000;
        std::string largeStr;
        for (size_t i = 0; i < max_cnt; ++i) {
            largeStr += "1";
            if (i < max_cnt-1) {
                largeStr += ",";
            }
        }

        size_t num_tokens;
        char** tokens = splitter.run(largeStr.c_str(), ",", &num_tokens);

        REQUIRE(num_tokens == max_cnt);
        for (size_t i = 0; i < num_tokens; ++i) {
            REQUIRE(std::string(tokens[i]) == "1");
        }

        splitter.free(tokens, num_tokens);
    }
}
