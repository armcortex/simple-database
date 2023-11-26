//
// Created by MCS51_M2 on 2023/11/19.
//
#include "../src/db_config.h"

#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <filesystem>

#include "../src/inputs.h"
#include "../src/prompt.h"
#include "io_redirect.h"
#include "../src/helper_functions.h"
#include "../src/cmd_functions.h"
#include "../src/database.h"


TEST_CASE("Check Commands", "[command]") {
    setvbuf(stdout, NULL, _IONBF, 0);

    SECTION("Basic input query") {
        // Init
        IORedirector redirector;
        prompt_buf_t *prompt_buf = new_prompt_buf();

        // Testing
        std::string query = stdin_write_data(redirector, prompt_buf, "select * from db\n");
        REQUIRE(std::string(prompt_buf->buf) == query);
        REQUIRE(prompt_buf->len == query.length());

        // Close
        free_prompt_buf(prompt_buf);
    }

    SECTION("Query state") {
        // Init
        IORedirector redirector;
        query_state_t *query_state = query_state_construct();
        query_state->init(query_state);
        prompt_buf_t *prompt_buf = new_prompt_buf();

        // Testing
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

        stdin_write_data(redirector, prompt_buf, "delete\n");
        check_commands(prompt_buf, query_state);
        REQUIRE(query_state->state == DELETE);

        stdin_write_data(redirector, prompt_buf, "use\n");
        check_commands(prompt_buf, query_state);
        REQUIRE(query_state->state == USE);

        stdin_write_data(redirector, prompt_buf, "select\n");
        check_commands(prompt_buf, query_state);
        REQUIRE(query_state->state == SELECT);

        // Close
        free_prompt_buf(prompt_buf);
        query_state->close(query_state);
    }

    SECTION("Create/delete database") {
        // Init
        IORedirector redirector;
        std::string db_name = "my_db";
        std::string db_file_path = WORKSPACE_PATH_FULL "/" + db_name + "/" + db_name + ".json";
        bool fileExists;

        create_folder((const char*)WORKSPACE_PATH_FULL);

        query_state_t *query_state = query_state_construct();
        query_state->init(query_state);
        prompt_buf_t *prompt_buf = new_prompt_buf();

        //Testing
        // Check create database file ok
        stdin_write_data(redirector, prompt_buf, "create database " + db_name + "\n");
        check_commands(prompt_buf, query_state);

        fileExists = std::filesystem::exists(db_file_path);
        REQUIRE(fileExists);

        // Check delete database file ok
        stdin_write_data(redirector, prompt_buf, "delete database " + db_name + "\n");
        check_commands(prompt_buf, query_state);

        fileExists = std::filesystem::exists(db_file_path);
        REQUIRE_FALSE(fileExists);

        // Close
        free_prompt_buf(prompt_buf);
        query_state->close(query_state);
    }

    SECTION("Create/delete table") {
        // Init
        IORedirector redirector;
        std::string db_name = "my_db";
        std::string table_name = "my_table";
        std::string db_folder = WORKSPACE_PATH_FULL "/" + db_name + "/";
        std::string db_file_path = db_folder + db_name + ".json";
        std::string table_file_path = db_folder + table_name + ".csv";
        bool fileExists;
        bool res;
        std::string read_str;

        query_state_t *query_state = query_state_construct();
        query_state->init(query_state);
        prompt_buf_t *prompt_buf = new_prompt_buf();

        // Create database
        stdin_write_data(redirector, prompt_buf, "create database " + db_name + "\n");
        check_commands(prompt_buf, query_state);

        // Testing
        // Not using `USE` command
        // Create table
        stdin_write_data(redirector, prompt_buf, "create table " + table_name + "\n");
        check_commands(prompt_buf, query_state);
        fileExists = std::filesystem::exists(table_file_path);
        REQUIRE_FALSE(fileExists);

        // Check not using 'USE` response
        read_str = redirector.read_stderr();
        res = compare_io_response_str(read_str, "Don't know what database to use, please use `USE` command to select database first \n");
        REQUIRE(res);

        // Use database
        stdin_write_data(redirector, prompt_buf, "use " + db_name + "\n");
        check_commands(prompt_buf, query_state);

        // Testing
        // Create table , not enough arguments
        redirector.flush();
        stdin_write_data(redirector, prompt_buf, "create table " + table_name + "\n");
        check_commands(prompt_buf, query_state);
        fileExists = std::filesystem::exists(table_file_path);
        REQUIRE_FALSE(fileExists);

        // Check `create table` command output string
        read_str = redirector.read_stderr();
        res = compare_io_response_str(read_str, "argument not enough: (<column name> <column type> ...)");
        REQUIRE(res);

        // Create table , arguments is not pair
        redirector.flush();
        stdin_write_data(redirector, prompt_buf, "create table " + table_name + " name STRING age \n");
        check_commands(prompt_buf, query_state);
        fileExists = std::filesystem::exists(table_file_path);
        REQUIRE_FALSE(fileExists);

        // Check `create table` command output string
        read_str = redirector.read_stderr();
        res = compare_io_response_str(read_str, "argument not enough: (<column name> <column type> ...)");
        REQUIRE(res);

        // Create table, enough arguments
        redirector.flush();
        stdin_write_data(redirector, prompt_buf, "create table " + table_name + " name STRING age INT height FLOAT \n");
        check_commands(prompt_buf, query_state);
        fileExists = std::filesystem::exists(table_file_path);
        REQUIRE(fileExists);

        // Check `create table` command output string
        read_str = redirector.read_stdout();
        res = compare_io_response_str(read_str, "Create table at: ../DB_DATA/my_db/my_table.csv \n");
        REQUIRE(res);

        // Delete table
//        std::filesystem::remove_all( db_folder);

//        stdin_write_data(redirector, prompt_buf, "delete table " + table_name + "\n");
//        check_commands(prompt_buf, query_state);
//        fileExists = std::filesystem::exists(table_file_path);
//        REQUIRE_FALSE(fileExists);


        // Close
        stdin_write_data(redirector, prompt_buf, "delete database " + db_name + "\n");
        check_commands(prompt_buf, query_state);
        fileExists = std::filesystem::exists(db_folder);
        REQUIRE_FALSE(fileExists);

        // Close
        free_prompt_buf(prompt_buf);
        query_state->close(query_state);

//        std::filesystem::remove_all( db_folder);
    }

    SECTION("Basic help command") {
        // Init
        IORedirector redirector;
        query_state_t *query_state = query_state_construct();
        query_state->init(query_state);
        prompt_buf_t *prompt_buf = new_prompt_buf();

        // Testing
        stdin_write_data(redirector, prompt_buf, "help\n");
        check_commands(prompt_buf, query_state);

        std::string s = redirector.read_stdout();
        s = filter_out_catch2_string(s);
        const std::string ref_str = "All Support commands: \n\t help: \n\t exit: \n\t create: \n\t use: \n\t drop: \n\t select: \n";
        REQUIRE(ref_str == s);

        // Close
        free_prompt_buf(prompt_buf);
        query_state->close(query_state);
    }

    SECTION("Create help command") {
        // Init
        IORedirector redirector;
        query_state_t *query_state = query_state_construct();
        query_state->init(query_state);
        prompt_buf_t *prompt_buf = new_prompt_buf();

        // Testing
        stdin_write_data(redirector, prompt_buf, "create help\n");
        check_commands(prompt_buf, query_state);

        std::string s = redirector.read_stdout();
        s = filter_out_catch2_string(s);
        const std::string ref_str = "Create sub-commands: \n\t database <database name> \n\t table <table name> (<column name> <column type> ...) \n";
        REQUIRE(ref_str == s);

        // Close
        free_prompt_buf(prompt_buf);
        query_state->close(query_state);
    }

    SECTION("Delete help command") {
        // Init
        IORedirector redirector;
        query_state_t *query_state = query_state_construct();
        query_state->init(query_state);
        prompt_buf_t *prompt_buf = new_prompt_buf();

        // Testing
        stdin_write_data(redirector, prompt_buf, "delete help\n");
        check_commands(prompt_buf, query_state);

        std::string s = redirector.read_stdout();
        s = filter_out_catch2_string(s);
        const std::string ref_str = "Delete sub-commands: \n\t database <database name> \n\t table <table name> \n";
        REQUIRE(ref_str == s);

        // Close
        free_prompt_buf(prompt_buf);
        query_state->close(query_state);
    }

    SECTION("Use help command") {
        // Init
        IORedirector redirector;
        query_state_t *query_state = query_state_construct();
        query_state->init(query_state);
        prompt_buf_t *prompt_buf = new_prompt_buf();

        // Testing
        stdin_write_data(redirector, prompt_buf, "use help\n");
        check_commands(prompt_buf, query_state);

        std::string s = redirector.read_stdout();
        s = filter_out_catch2_string(s);
        const std::string ref_str = "Use sub-commands: \n\t <database name> \n";
        REQUIRE(ref_str == s);

        // Close
        free_prompt_buf(prompt_buf);
        query_state->close(query_state);
    }

    SECTION("Use database command") {
        // Init
        IORedirector redirector;
        query_state_t *query_state = query_state_construct();
        query_state->init(query_state);
        prompt_buf_t *prompt_buf = new_prompt_buf();
        std::string db_name = "my_db";
        std::string not_exist_db_name = "not_exist_db";
        bool res;
        std::string read_str;

        // Testing
        // Create database
        stdin_write_data(redirector, prompt_buf, "create database " + db_name + "\n");
        check_commands(prompt_buf, query_state);

        // Check `CREATE` command output string
        read_str = redirector.read_stdout();
        res = compare_io_response_str(read_str, "Create database at: ../DB_DATA/my_db/my_db.json \n");
        REQUIRE(res);

        // Use database
        stdin_write_data(redirector, prompt_buf, "use " + db_name + "\n");
        check_commands(prompt_buf, query_state);

        current_db_t *current_db =  get_current_db();
        std::string current_db_name = current_db->name;
        REQUIRE(current_db_name == db_name);

        // Check `USE` command output string
        read_str = redirector.read_stdout();
        res = compare_io_response_str(read_str, "Using database: my_db \n");
        REQUIRE(res);

        // Delete database
        stdin_write_data(redirector, prompt_buf, "delete database " + db_name + "\n");
        check_commands(prompt_buf, query_state);

        // Check `DELETE` command output string
        read_str = redirector.read_stdout();
        res = compare_io_response_str(read_str, "Delete database: ../DB_DATA/my_db/my_db.json \n");
        REQUIRE(res);

        // Use none exist database
        redirector.flush();
        stdin_write_data(redirector, prompt_buf, "use " + not_exist_db_name + "\n");
        check_commands(prompt_buf, query_state);

        // Check `USE` command output string
        read_str = redirector.read_stderr();
        res = compare_io_response_str(read_str, "Database not_exist_db not exist \n");
        REQUIRE(res);

        // Close
        free_prompt_buf(prompt_buf);
        query_state->close(query_state);
    }
}