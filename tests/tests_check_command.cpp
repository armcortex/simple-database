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
    parse_commands(prompt_buf, query_state);
    execute_commands(query_state);
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

        cmd_str = "list\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        REQUIRE(query_state->state == LIST);
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

    SECTION("Help info 1: without sub-command") {
        // Init
        IORedirector redirector;

        // Testing
        cmd_str = "help\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        ref_str = "This is basic_sub_help()\n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);
    }

    SECTION("Help info 2: correct sub-command (a_fn)") {
        // Init
        IORedirector redirector;

        // Testing
        cmd_str = "help a_fn\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        ref_str = "This is basic_sub_a_fn()\n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);
    }

    SECTION("Help info 3: correct sub-command (b_fn)") {
        // Init
        IORedirector redirector;

        // Testing
        cmd_str = "help b_fn\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        ref_str = "This is basic_sub_b_fn()\n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);
    }

    SECTION("Help info 4: wrong sub-command") {
        // Init
        IORedirector redirector;

        // Testing
        cmd_str = "help b_fb\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        ref_str = "This is basic_sub_help()\n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);
    }


#if 0
    SECTION("Basic help command") {
        // Init
        IORedirector redirector;

        // Testing
        cmd_str = "help\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        ref_str = "All Support commands: \n\t help: \n\t exit: \n\t create: \n\t use: \n\t delete: \n\t select: \n\t list: \n";
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

    SECTION("List help command") {
        // Init
        IORedirector redirector;

        // Testing
        cmd_str = "list help\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        ref_str = "List sub-commands: \n\t database <database_name> \n\t table <table_name> \n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);
    }
#endif
    // Close
    free_prompt_buf(prompt_buf);
    query_state->close(query_state);
}

TEST_CASE("Create command", "[command]") {
    setvbuf(stdout, nullptr, _IONBF, 0);

    std::string cmd_str;
    std::string query_res;
    std::string read_str;
    std::string read_err_str;
    std::string ref_str;
    std::string ref_err_str;
    bool fileExists;
    bool res;

    const std::string db_name = "my_db";
    const std::string db_folder = WORKSPACE_PATH_FULL "/" + db_name + "/";
    const std::string db_file_path = WORKSPACE_PATH_FULL "/" + db_name + "/" + db_name + ".json";

    const std::string table_name = "my_table";
    const std::string table_file_path = db_folder + table_name + ".csv";

    // Init
    query_state_t *query_state = query_state_construct();
    query_state->init(query_state);
    prompt_buf_t *prompt_buf = new_prompt_buf();

    SECTION("Create help info 1: without sub-command") {
        // Init
        IORedirector redirector;

        // Testing
        cmd_str = "create\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        read_err_str = redirector.read_stderr();

        ref_str = "Create sub-commands: \n\t database <database name> \n\t table <table name> (<column name> <column type> ...) \n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);

        ref_err_str = "";
        res = compare_io_response_str(read_err_str, ref_err_str);
        REQUIRE(res);
    }

    SECTION("Create help info 2: correct sub-command") {
        // Init
        IORedirector redirector;

        // Testing
        cmd_str = "create help\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        read_err_str = redirector.read_stderr();

        ref_str = "Create sub-commands: \n\t database <database name> \n\t table <table name> (<column name> <column type> ...) \n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);

        ref_err_str = "";
        res = compare_io_response_str(read_err_str, ref_err_str);
        REQUIRE(res);
    }

    SECTION("Create help info 3: wrong sub-command") {
        // Init
        IORedirector redirector;

        // Testing
        cmd_str = "create non_exist_cmd\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        read_err_str = redirector.read_stderr();

        ref_str = "Create sub-commands: \n\t database <database name> \n\t table <table name> (<column name> <column type> ...) \n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);

        ref_err_str = "";
        res = compare_io_response_str(read_err_str, ref_err_str);
        REQUIRE(res);
    }

    SECTION("Create database 1: correct") {
        // Init
        IORedirector redirector;

        // Testing
        cmd_str = "create database " + db_name + "\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        read_err_str = redirector.read_stderr();

        ref_str = "Create database at: ../DB_DATA/my_db/my_db.json \n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);

        ref_err_str = "";
        res = compare_io_response_str(read_err_str, ref_err_str);
        REQUIRE(res);

        fileExists = std::filesystem::exists(db_file_path);
        remove(db_file_path.c_str());
        remove_folder(db_folder.c_str());
        REQUIRE(fileExists);
    }

    SECTION("Create database 2: wrong argument count") {
        // Init
        IORedirector redirector;

        // Testing
        cmd_str = "create database " + db_name + " wrong_arg\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        read_err_str = redirector.read_stderr();

        ref_str = "Create sub-commands: \n\t database <database name> \n\t table <table name> (<column name> <column type> ...) \n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);

        ref_err_str = "Wrong arguments\n";
        res = compare_io_response_str(read_err_str, ref_err_str);
        REQUIRE(res);

        fileExists = std::filesystem::exists(db_file_path);
        REQUIRE_FALSE(fileExists);
    }

#if 0
    SECTION("Create table") {
        // Init
        IORedirector redirector;

        // Testing
        cmd_str = "create non_exist_cmd\n";
        query_res = execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        read_err_str = redirector.read_stderr();

        ref_str = "This is basic_sub_help()\n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);

        ref_err_str = "";
        res = compare_io_response_str(read_err_str, ref_err_str);
        REQUIRE(res);
    }
#endif

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
        const current_db_t *current_db = get_current_db();
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
    // bool res;

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
    // bool fileExists;
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
    std::string read_err_str;
    std::string ref_str;
    // bool fileExists;
    bool res;

    const std::string db_name = "my_db";
    const std::string db_folder = WORKSPACE_PATH_FULL "/" + db_name + "/";
    const std::string db_file_path = db_folder + db_name + ".json";

    const std::string table_name = "my_table";
    const std::string table_file_path = db_folder + table_name + ".csv";

    const std::string not_exist_table_name = "none_my_table";

    std::vector<std::string> insert_datas = {
            "John 30 170",
            "Jane 25 165",
            "Alice 28 180",
            "Bob 31 173",
            "Charlie 29 160"
    };

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
        std::string cmd_str_base = "insert " + table_name + " values ";
        for (const auto& data : insert_datas) {
            cmd_str = cmd_str_base + data + "\n";
            execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        }

        // Testing
        redirector.flush();
        cmd_str = "select name,age,height from " + table_name + " where age < 35\n";    // For now need `where` command

        execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        ref_str = "name,age,height\nJohn,30,170\nJane,25,165\nAlice,28,180\nBob,31,173\nCharlie,29,160\n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);

        // Close
        cmd_str = "delete database " + db_name + "\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);
    }

    SECTION("Select and where table data 1: select name,age") {
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
        std::string cmd_str_base = "insert " + table_name + " values ";
        for (const auto& data : insert_datas) {
            cmd_str = cmd_str_base + data + "\n";
            execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        }

        // Testing
        redirector.flush();
        cmd_str = "select name,age from " + table_name + " where age < 29 and ( name = Jane or name = Alice )\n";

        execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        read_err_str = redirector.read_stderr();

        ref_str = "name,age\nJane,25\nAlice,28\n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);

        // Close
        cmd_str = "delete database " + db_name + "\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);
    }

    SECTION("Select and where table data 2: select age,height") {
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
        std::string cmd_str_base = "insert " + table_name + " values ";
        for (const auto& data : insert_datas) {
            cmd_str = cmd_str_base + data + "\n";
            execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        }

        // Testing
        redirector.flush();
        cmd_str = "select age,height from " + table_name + " where ( age < 29 ) and ( height < 190.2 )\n";

        execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        read_err_str = redirector.read_stderr();

        ref_str = "name,age\n25,165\n28,180\n";
        res = compare_io_response_str(read_str, ref_str);
        REQUIRE(res);

        // Close
        cmd_str = "delete database " + db_name + "\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);
    }

    SECTION("Wrong select column name and where table data") {
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
        std::string cmd_str_base = "insert " + table_name + " values ";
        for (const auto& data : insert_datas) {
            cmd_str = cmd_str_base + data + "\n";
            execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        }

        // Testing
        redirector.flush();
        cmd_str = "select aa,bb from " + table_name + " where age < 29\n";

        execute_cmd(redirector, prompt_buf, query_state, cmd_str);
        read_str = redirector.read_stdout();
        read_err_str = redirector.read_stderr();

        ref_str = "Column name not matched: aa to (name, age, height) \nColumn name not found \n";  // not include `age<29` condition
        res = compare_io_response_str(read_err_str, ref_str);
        REQUIRE(res);

        // Close
        cmd_str = "delete database " + db_name + "\n";
        execute_cmd(redirector, prompt_buf, query_state, cmd_str);
    }

    // Close
    free_prompt_buf(prompt_buf);
    query_state->close(query_state);
}
