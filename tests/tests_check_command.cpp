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
#include <fstream>
#include <nlohmann/json.hpp>

#include "../src/inputs.h"
#include "../src/prompt.h"
#include "io_redirect.h"
#include "../src/helper_functions.h"
#include "../src/cmd_functions.h"
#include "../src/database.h"


std::string execute_cmd(IORedirector &rd, prompt_buf_t *prompt_buf, query_state_t *query_state, std::string cmd_str) {
    std::string query_res = stdin_write_data(rd, prompt_buf, cmd_str);
    check_commands(prompt_buf, query_state);
    return query_res;
}

TEST_CASE("Basic query Commands", "[command]") {
    setvbuf(stdout, nullptr, _IONBF, 0);

    std::string cmd_str;
    std::string query_res;

    // Init
    query_state_t *query_state = query_state_construct();
    query_state->init(query_state);
    prompt_buf_t *prompt_buf = new_prompt_buf();

    SECTION("Basic input query") {
        // Init
        IORedirector redirector;

        // Testing
        cmd_str = "select * from db\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        REQUIRE(std::string(prompt_buf->buf) == query_res);
        REQUIRE(prompt_buf->len == query_res.length());
    }

    SECTION("Query state") {
        // Init
        IORedirector redirector;

        // Testing
        REQUIRE(query_state->state == INIT);

        cmd_str = "exit\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        REQUIRE(query_state->state == EXIT);

        cmd_str = "help\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        REQUIRE(query_state->state == HELP);

        cmd_str = "create\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        REQUIRE(query_state->state == CREATE);

        cmd_str = "delete\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        REQUIRE(query_state->state == DELETE);

        cmd_str = "use\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        REQUIRE(query_state->state == USE);

        cmd_str = "select\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        REQUIRE(query_state->state == SELECT);

        cmd_str = "insert\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        REQUIRE(query_state->state == INSERT);
    }

    // Close
    free_prompt_buf(prompt_buf);
    query_state->close(query_state);
}


TEST_CASE("Help command info", "[command]") {
    setvbuf(stdout, nullptr, _IONBF, 0);

    std::string cmd_str;
    std::string query_res;
    std::string read_str;
    std::string ref_str;
    bool res;

    // Init
    query_state_t *query_state = query_state_construct();
    query_state->init(query_state);
    prompt_buf_t *prompt_buf = new_prompt_buf();

    SECTION("Basic help command") {
        // Init
        IORedirector redirector;

        // Testing
        cmd_str = "help\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        ref_str = "All Support commands: \n\t help: \n\t exit: \n\t create: \n\t use: \n\t drop: \n\t select: \n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);
    }

    SECTION("Create help command") {
        // Init
        IORedirector redirector;

        // Testing
        cmd_str = "create help\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        ref_str = "Create sub-commands: \n\t database <database name> \n\t table <table name> (<column name> <column type> ...) \n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);
    }

    SECTION("Delete help command") {
        // Init
        IORedirector redirector;

        // Testing
        cmd_str = "delete help\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        ref_str = "Delete sub-commands: \n\t database <database name> \n\t table <table name> \n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);
    }

    SECTION("Use help command") {
        // Init
        IORedirector redirector;

        // Testing
        cmd_str = "use help\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        ref_str = "Use sub-commands: \n\t <database name> \n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);
    }

    SECTION("Insert help command") {
        // Init
        IORedirector redirector;

        // Testing
        cmd_str = "insert help\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        ref_str = "insert <table_name> values <value1,value2,value3,...>\n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);
    }

    SECTION("Select help command") {
        // Init
        IORedirector redirector;

        // Testing
        cmd_str = "select help\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        ref_str = "select <column_names> from <table_name> (where <condition> ...) \n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);
    }

    // Close
    free_prompt_buf(prompt_buf);
    query_state->close(query_state);
}


TEST_CASE("Commands behavior", "[command]") {
    setvbuf(stdout, nullptr, _IONBF, 0);

    std::string cmd_str;
    std::string query_res;
    std::string read_str;
    std::string ref_str;
    bool fileExists;
    bool res;

    const std::string db_name = "my_db";
    const std::string db_folder = WORKSPACE_PATH_FULL "/" + db_name + "/";
    const std::string db_file_path = WORKSPACE_PATH_FULL "/" + db_name + "/" + db_name + ".json";

    const std::string table_name = "my_table";
    const std::string table_file_path = db_folder + table_name + ".csv";

    const std::string not_exist_db_name = "not_exist_db";


    // Init
    query_state_t *query_state = query_state_construct();
    query_state->init(query_state);
    prompt_buf_t *prompt_buf = new_prompt_buf();

    SECTION("Create/delete database") {
        // Init
        IORedirector redirector;

        create_folder((const char*)WORKSPACE_PATH_FULL);

        //Testing
        // Check create database file ok
        cmd_str = "create database " + db_name + "\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        fileExists = std::filesystem::exists(db_file_path);
        REQUIRE(fileExists);

        // Check delete database file ok
        cmd_str = "delete database " + db_name + "\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        fileExists = std::filesystem::exists(db_file_path);
        REQUIRE_FALSE(fileExists);
    }

    SECTION("Create/delete table") {
        // Init
        IORedirector redirector;

        // Create database
        redirector.flush();
        cmd_str = "create database " + db_name + "\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);


        // Testing
        // Not using `USE` command
        // Create table
        redirector.flush();
        cmd_str = "create table " + table_name + "\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        fileExists = std::filesystem::exists(table_file_path);
        REQUIRE_FALSE(fileExists);

        // Check not using 'USE` response
        read_str = redirector.read_stderr();
        ref_str = "Don't know what database to use, please use `USE` command to select database first \n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);

        // Use database
        cmd_str = "use " + db_name + "\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);


        // Testing
        // Create table , not enough arguments
        redirector.flush();
        cmd_str = "create table " + table_name + "\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        fileExists = std::filesystem::exists(table_file_path);
        REQUIRE_FALSE(fileExists);

        // Check `create table` command output string
        read_str = redirector.read_stderr();
        ref_str = "argument not enough: (<column name> <column type> ...)";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);

        // Create table , arguments is not pair
        redirector.flush();
        cmd_str = "create table " + table_name + " name STRING age \n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        fileExists = std::filesystem::exists(table_file_path);
        REQUIRE_FALSE(fileExists);

        // Check `create table` command output string
        read_str = redirector.read_stderr();
        ref_str = "argument not enough: (<column name> <column type> ...)";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);

        // Create table, enough arguments
        redirector.flush();
        cmd_str = "create table " + table_name + " name STRING age INT height FLOAT \n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        fileExists = std::filesystem::exists(table_file_path);
        REQUIRE(fileExists);

        // Check `create table` command output string
        read_str = redirector.read_stdout();
        ref_str = "Create table at: ../DB_DATA/my_db/my_table.csv \n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);

        // Delete database, all tables will be deleted as well
        redirector.flush();
        cmd_str = "delete database " + db_name + "\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        fileExists = std::filesystem::exists(db_folder);
        REQUIRE_FALSE(fileExists);
    }

    SECTION("Use database command") {
        // Init
        IORedirector redirector;

        // Testing
        // Create database
        // Check `CREATE` command output string
        redirector.flush();
        cmd_str = "create database " + db_name + "\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        ref_str = "Create database at: ../DB_DATA/my_db/my_db.json \n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);

        // Use database
        // Check `USE` command output string
        redirector.flush();
        cmd_str = "use " + db_name + "\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        current_db_t *current_db =  get_current_db();
        std::string current_db_name = current_db->name;
        REQUIRE(current_db_name == db_name);

        read_str = redirector.read_stdout();
        ref_str = "Using database: my_db \n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);


        // Delete database
        // Check `DELETE` command output string
        redirector.flush();
        cmd_str = "delete database " + db_name + "\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        ref_str = "Delete database: ../DB_DATA/my_db/my_db.json \n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);


        // Use none exist database
        // Check `USE` command output string
        redirector.flush();
        cmd_str = "use " + not_exist_db_name + "\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stderr();
        ref_str = "Database not_exist_db not exist \n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);
    }

    // Close
    free_prompt_buf(prompt_buf);
    query_state->close(query_state);
}

TEST_CASE("Create Table JSON Test", "[create_table]") {
    setvbuf(stdout, nullptr, _IONBF, 0);

    std::string cmd_str;
    std::string query_res;
    std::string read_str;
    bool fileExists;
    bool res;

    const std::string db_name = "my_db";
    const std::string db_folder = WORKSPACE_PATH_FULL "/" + db_name + "/";
    const std::string db_file_path = db_folder + db_name + ".json";

    const std::string table_name = "my_table";
    const std::string table_file_path = db_folder + table_name + ".csv";


    // Init
    query_state_t *query_state = query_state_construct();
    query_state->init(query_state);
    prompt_buf_t *prompt_buf = new_prompt_buf();

    SECTION("Creating table and checking JSON structure") {
        // Init
        IORedirector redirector;

        // Testing
        // Create database
        cmd_str = "create database " + db_name + "\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        fileExists = std::filesystem::exists(db_file_path);
        REQUIRE(fileExists);

        // Use database
        cmd_str = "use " + db_name + "\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);

        // Create table
        cmd_str = "create table " + table_name + " name STRING age INT height FLOAT \n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        fileExists = std::filesystem::exists(table_file_path);
        REQUIRE(fileExists);

        // Check database meta data json file
        std::ifstream f(db_file_path);
        nlohmann::json db_json = nlohmann::json::parse(f);
        db_json.erase("time");
        db_json.erase("timestamp");
        std::string db_json_str = db_json.dump(2);

        nlohmann::json ref_db_json = {
            {"table_cnt", 1},
            {"tables", {{
                {"table_name", "my_table"},
                {"data_cnt", 0},
                {"columns", {
                    {{"column_name", "name"}, {"type", "STRING"}},
                    {{"column_name", "age"}, {"type", "INT"}},
                    {{"column_name", "height"}, {"type", "FLOAT"}}
                }}
            }}}
        };
        std::string ref_db_json_str = ref_db_json.dump(2);
        REQUIRE(db_json == ref_db_json);

        // check table csv file
        std::ifstream file(table_file_path);
        std::string table_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        std::string ref_table_content = "name,age,height\n";
        REQUIRE(table_content == ref_table_content);

        // Delete database
        cmd_str = "delete database " + db_name + "\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        fileExists = std::filesystem::exists(db_folder);
        REQUIRE_FALSE(fileExists);
    }

    // Close
    free_prompt_buf(prompt_buf);
    query_state->close(query_state);
}

TEST_CASE("Insert Data Test", "[insert]") {
    setvbuf(stdout, nullptr, _IONBF, 0);

    std::string cmd_str;
    std::string read_str;
    bool fileExists;
    bool res;

    const std::string db_name = "my_db";
    const std::string db_folder = WORKSPACE_PATH_FULL "/" + db_name + "/";
    const std::string db_file_path = db_folder + db_name + ".json";

    const std::string table_name = "my_table";
    const std::string table_file_path = db_folder + table_name + ".csv";


    // Init
    query_state_t *query_state = query_state_construct();
    query_state->init(query_state);
    prompt_buf_t *prompt_buf = new_prompt_buf();

    SECTION("Insert table not found") {
        // Init
        IORedirector redirector;
        cmd_str = "create database " + db_name + "\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);

        cmd_str = "use " + db_name + "\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);

        cmd_str = "create table " + table_name + " name STRING age INT height FLOAT \n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);

        // Testing
        redirector.flush();
        std::string not_exist_table_name = "none_my_table";
        std::string cmd_str = "insert " + not_exist_table_name + " values 1,2,3\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stderr();
        res = compare_io_response_str(read_str, "Table " + not_exist_table_name + " not found\n");
        REQUIRE(res);

        // Close
        cmd_str = "delete database " + db_name + "\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);
    }

    SECTION("Inserting data into table") {
        // Init
        IORedirector redirector;
        cmd_str = "create database " + db_name + "\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);

        cmd_str = "use " + db_name + "\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);

        cmd_str = "create table " + table_name + " name STRING age INT height FLOAT \n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);

        // Testing
        std::vector<std::string> insert_datas = {
                "John 30 170",
                "Jane 25 165",
                "Alice 28 180",
                "Bob 31 173",
                "Charlie 29 160"
        };

        std::string cmd_str_base = "insert " + table_name + " values ";
        for (const auto& data : insert_datas) {
            cmd_str = cmd_str_base + data + "\n";
            execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        }

        // check table data
        std::ifstream file(table_file_path);
        std::string table_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        std::string ref_table_content = "name,age,height\nJohn,30,170\nJane,25,165\nAlice,28,180\nBob,31,173\nCharlie,29,160\n";
        REQUIRE(table_content == ref_table_content);

        // check database meta
        std::ifstream f(db_file_path);
        nlohmann::json db_json = nlohmann::json::parse(f);
        std::string db_json_str = db_json.dump(2);
        REQUIRE(db_json["tables"][0]["table_name"] == table_name);
        REQUIRE(db_json["tables"][0]["data_cnt"] == insert_datas.size());

        // Close
        cmd_str = "delete database " + db_name + "\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);
    }

    SECTION("Inserting data into table multiple times") {
        // Init
        IORedirector redirector;
        cmd_str = "create database " + db_name + "\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);

        cmd_str = "use " + db_name + "\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);

        cmd_str = "create table " + table_name + " name STRING age INT height FLOAT \n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);

        // Testing
        const int repeat_times = 3;
        std::vector<std::string> insert_datas;
        for (int i=0; i<repeat_times; i++) {
            insert_datas = {
                    "John 30 170",
                    "Jane 25 165",
                    "Alice 28 180",
                    "Bob 31 173",
                    "Charlie 29 160"
            };

            std::string cmd_str_base = "insert " + table_name + " values ";
            for (const auto &data: insert_datas) {
                cmd_str = cmd_str_base + data + "\n";
                execute_cmd(redirector, prompt_buf, query_state, cmd_str);
            }
        }

        // check table data
        std::ifstream file(table_file_path);
        std::string table_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        std::string ref_base = "name,age,height\n";
        std::string ref_data_base = "John,30,170\nJane,25,165\nAlice,28,180\nBob,31,173\nCharlie,29,160\n";
        std::string ref_data;
        for (int i=0; i<repeat_times; i++) {
            ref_data += ref_data_base;
        }
        std::string ref_table_content = ref_base + ref_data;
        REQUIRE(table_content == ref_table_content);

        // check database meta
        std::ifstream f(db_file_path);
        nlohmann::json db_json = nlohmann::json::parse(f);
        std::string db_json_str = db_json.dump(2);
        REQUIRE(db_json["tables"][0]["table_name"] == table_name);
        REQUIRE(db_json["tables"][0]["data_cnt"] == (insert_datas.size() * repeat_times));

        // Close
        cmd_str = "delete database " + db_name + "\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);
    }

    // Close
    free_prompt_buf(prompt_buf);
    query_state->close(query_state);
}

TEST_CASE("Select table Test", "[select]") {
    setvbuf(stdout, nullptr, _IONBF, 0);

    std::string cmd_str;
    std::string query_res;
    std::string read_str;
    std::string ref_str;
    bool fileExists;
    bool res;

    const std::string db_name = "my_db";
    const std::string db_folder = WORKSPACE_PATH_FULL "/" + db_name + "/";
    const std::string db_file_path = db_folder + db_name + ".json";

    const std::string table_name = "my_table";
    const std::string table_file_path = db_folder + table_name + ".csv";

    const std::string not_exist_table_name = "none_my_table";

    // Init
    query_state_t *query_state = query_state_construct();
    query_state->init(query_state);
    prompt_buf_t *prompt_buf = new_prompt_buf();

    SECTION("Select table not found") {
        // Init
        IORedirector redirector;

        // Create database
        cmd_str = "create database " + db_name + "\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);

        // Use database
        cmd_str = "use " + db_name + "\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);


        // Testing
        redirector.flush();
        cmd_str = "select * from " + not_exist_table_name + " \n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stderr();
        ref_str = "Table none_my_table not found\n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);

        // Close
        cmd_str = "delete database " + db_name + "\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);
    }

    SECTION("Select table data") {
        // Init
        IORedirector redirector;

        // Create database
        cmd_str = "create database " + db_name + "\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);

        // Use database
        cmd_str = "use " + db_name + "\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);

        // Create table
        cmd_str = "create table " + table_name + " name STRING age INT height FLOAT \n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);

        // Insert data
        std::vector<std::string> insert_datas = {
                "John 30 170",
                "Jane 25 165",
                "Alice 28 180",
                "Bob 31 173",
                "Charlie 29 160"
        };

        std::string cmd_str_base = "insert " + table_name + " values ";
        for (const auto& data : insert_datas) {
            cmd_str = cmd_str_base + data + "\n";
            execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        }

        // Testing
        redirector.flush();
//        cmd_str = "select name,age,height from " + table_name + ";\n";
        cmd_str = "select name,age,height from " + table_name + " where age < 29\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        ref_str = "name,age,height\nJohn,30,170\nJane,25,165\nAlice,28,180\nBob,31,173\nCharlie,29,160\n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);

        // Close
        cmd_str = "delete database " + db_name + "\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);
    }

    SECTION("Select and where table data") {
        // Init
        IORedirector redirector;

        // Create database
        cmd_str = "create database " + db_name + "\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);

        // Use database
        cmd_str = "use " + db_name + "\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);

        // Create table
        cmd_str = "create table " + table_name + " name STRING age INT height FLOAT \n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);

        // Insert data
        std::vector<std::string> insert_datas = {
                "John 30 170",
                "Jane 25 165",
                "Alice 28 180",
                "Bob 31 173",
                "Charlie 29 160"
        };

        std::string cmd_str_base = "insert " + table_name + " values ";
        for (const auto& data : insert_datas) {
            cmd_str = cmd_str_base + data + "\n";
            execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        }

        // Testing
        redirector.flush();
        cmd_str = "select name,age from " + table_name + " where age < 29\n";
//        cmd_str = "select name,height from " + table_name + " where age < 29\n";

        execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        ref_str = "name,age\nJohn,30\nJane,25\nAlice,28\nBob,31\nCharlie,29\n";  // not include `age<29` condition
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);

        // Close
        cmd_str = "delete database " + db_name + "\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);
    }

    // Close
    free_prompt_buf(prompt_buf);
    query_state->close(query_state);
}
