//
// Created by MCS51_M2 on 2023/11/19.
//

#include <catch2/catch_test_macros.hpp>
#include <unistd.h>
#include <cstring>
#include <cstdio>

#include "../src/inputs.h"
#include "../src/prompt.h"
#include "stdin_redirect.h"

TEST_CASE("Check Commands", "[command]") {
    StdinRedirector redirector;

    SECTION("Simple Command Test 1") {
        prompt_buf_t *prompt_buf = new_prompt_buf();

        std::string query = stdin_write_data(redirector, prompt_buf, "select * from db\n");
        REQUIRE(std::string(prompt_buf->buf) == query);
        REQUIRE(prompt_buf->len == query.length());

        free_prompt_buf(prompt_buf);
    }

    SECTION("Simple Command Test 2") {
        query_state_t *query_state = query_state_construct();
        query_state->init(query_state);

        prompt_buf_t *prompt_buf = new_prompt_buf();

        REQUIRE(query_state->state == INIT);

        stdin_write_data(redirector, prompt_buf, "exit\n");
        check_commands(prompt_buf, query_state);
        REQUIRE(query_state->state == EXIT);

        stdin_write_data(redirector, prompt_buf, "create\n");
        check_commands(prompt_buf, query_state);
        REQUIRE(query_state->state == CREATE);

        stdin_write_data(redirector, prompt_buf, "drop\n");
        check_commands(prompt_buf, query_state);
        REQUIRE(query_state->state == DROP);

        stdin_write_data(redirector, prompt_buf, "use\n");
        check_commands(prompt_buf, query_state);
        REQUIRE(query_state->state == USE);

        stdin_write_data(redirector, prompt_buf, "select\n");
        check_commands(prompt_buf, query_state);
        REQUIRE(query_state->state == SELECT);

        free_prompt_buf(prompt_buf);
        query_state->close(query_state);
    }
}
