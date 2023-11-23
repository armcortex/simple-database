//
// Created by MCS51_M2 on 2023/11/19.
//
#include "../src/db_config.h"

#include <catch2/catch_test_macros.hpp>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <filesystem>

#include "../src/inputs.h"
#include "../src/prompt.h"
#include "stdin_redirect.h"
#include "../src/helper_functions.h"
#include "../src/cmd_functions.h"

TEST_CASE("Check Commands", "[command]") {
    setvbuf(stdout, NULL, _IONBF, 0);
    StdinRedirector redirector;

    SECTION("Basic input query") {
        prompt_buf_t *prompt_buf = new_prompt_buf();

        std::string query = stdin_write_data(redirector, prompt_buf, "select * from db\n");
        REQUIRE(std::string(prompt_buf->buf) == query);
        REQUIRE(prompt_buf->len == query.length());

        free_prompt_buf(prompt_buf);
    }

    SECTION("Query state") {
        query_state_t *query_state = query_state_construct();
        query_state->init(query_state);

        prompt_buf_t *prompt_buf = new_prompt_buf();

        REQUIRE(query_state->state == INIT);

        stdin_write_data(redirector, prompt_buf, "exit\n");
        check_commands(prompt_buf, query_state);
        REQUIRE(query_state->state == EXIT);

        stdin_write_data(redirector, prompt_buf, "help\n");
        check_commands(prompt_buf, query_state);
        REQUIRE(query_state->state == HELP);

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

    SECTION("Create/delete database") {
        std::string db_name = "my_db";
        std::string db_file_path = WORKSPACE_PATH_FULL "/" + db_name + "/" + db_name + ".txt";
        bool fileExists;

        create_folder((const char*)WORKSPACE_PATH_FULL);

        query_state_t *query_state = query_state_construct();
        query_state->init(query_state);

        prompt_buf_t *prompt_buf = new_prompt_buf();

        // Check create database file ok
        stdin_write_data(redirector, prompt_buf, "create database " + db_name + "\n");
        check_commands(prompt_buf, query_state);

        fileExists = std::filesystem::exists(db_file_path);
        REQUIRE(fileExists);

        // Check delete database file ok
        delete_database((const char*)db_file_path.c_str());
        fileExists = std::filesystem::exists(db_file_path);
        REQUIRE_FALSE(fileExists);

        free_prompt_buf(prompt_buf);
        query_state->close(query_state);
    }
}