//
// Created by MCS51_M2 on 2023/11/16.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prompt.h"
#include "helper_functions.h"
#include "cmd_functions.h"
#include "database.h"
#include "parser.h"

void print_prompt() {
    current_db_t *db = get_current_db();
    fprintf(stdout, "%s > ", db->name);
    // TODO: fix bug, when delete current using database, the prompt still showing it, not clean it.
}

prompt_buf_t* new_prompt_buf() {
    prompt_buf_t *prompt_buf = (prompt_buf_t*)malloc(sizeof(prompt_buf_t));
    if (prompt_buf == NULL) {
        DB_ASSERT(!"Failed to allocate memory.\n");
    }

    prompt_buf->buf = NULL;
    prompt_buf->len = 0;
    return prompt_buf;
}

void free_prompt_buf(prompt_buf_t *prompt_buf) {
    if (prompt_buf->buf != NULL) {
        free(prompt_buf->buf);
        prompt_buf->buf = NULL;
    }

    if (prompt_buf != NULL) {
        free(prompt_buf);
        prompt_buf = NULL;
    }
}

void check_commands(prompt_buf_t *prompt_buf, query_state_t *query_state) {
    splitter_t splitter = split_construct();
    size_t num_tokens;
    char** cmds = splitter.run(prompt_buf->buf, " ", &num_tokens);

    // Exit
    if (strncmp(cmds[0], "exit", 4) == 0) {
        query_state->state = EXIT;
    }
    else if (strncmp(cmds[0], "help", 4) == 0) {
        query_state->state = HELP;
        basic_command_info();
    }
    // Create
    else if (strncmp(cmds[0], "create", 6) == 0) {
        query_state->state = CREATE;

        // Sub-commands
        if (cmds[1] != NULL) {
            // Help: list all support sub commands
            if (strncmp(cmds[1], "-h", 2) == 0 ||
                strncmp(cmds[1], "help", 4) == 0) {
                create_command_info();
            }
            else if (strncmp(cmds[1], "database", 8) == 0) {
                // Create database folder
                const char *db_folder_name = str_concat("%s/%s", WORKSPACE_PATH_FULL, cmds[2]);
                create_folder(db_folder_name);

                // Create database file
                const char *db_filename = str_concat("%s/%s/%s.json", WORKSPACE_PATH_FULL, cmds[2], cmds[2]);
                create_database(db_filename);
                fprintf(stdout, "Create database at: %s \n", db_filename);
            }
            else if (strncmp(cmds[1], "table", 5) == 0) {
                current_db_t *curr_db =  get_current_db();
                if (curr_db->len == 0) {
                    fprintf(stderr, "Don't know what database to use, please use `USE` command to select database first \n");
                }
                // Make sure command contain `create table table_name` and `column_name + type` pair
                else if ((num_tokens < 5) || ((num_tokens-3) % 2 != 0)) {
                    fprintf(stderr, "argument not enough: (<column name> <column type> ...)");
                }
                else {
                    const char *table_filename = str_concat("%s/%s/%s.csv", WORKSPACE_PATH_FULL, curr_db->name, cmds[2]);
                    create_table(table_filename, cmds[2], &cmds[3], num_tokens-3);
                    fprintf(stdout, "Create table at: %s \n", table_filename);
                }
            }
            else {
                fprintf(stderr, "Unrecognized command '%s' \n\n", prompt_buf->buf);
            }
        }
        else {
            create_command_info();
        }
    }
    // Delete
    else if (strncmp(cmds[0], "delete", 4) == 0) {
        query_state->state = DELETE;

        // Sub-commands
        if (cmds[1] != NULL) {
            // Help: list all support sub commands
            if (strncmp(cmds[1], "-h", 2) == 0 ||
                strncmp(cmds[1], "help", 4) == 0) {
                delete_command_info();
            }
            else if (strncmp(cmds[1], "database", 8) == 0) {
                // Delete database file
                const char *db_filename = str_concat("%s/%s/%s.json", WORKSPACE_PATH_FULL, cmds[2], cmds[2]);
                delete_database(db_filename);

                // Delete folder
                const char *db_folder_name = str_concat("%s/%s", WORKSPACE_PATH_FULL, cmds[2]);
                delete_table_all(db_folder_name);
                remove_folder(db_folder_name);
            }
            else {
                fprintf(stderr, "Unrecognized command '%s' \n\n", prompt_buf->buf);
            }
        }
    }
    // Use database
    else if (strncmp(cmds[0], "use", 3) == 0) {
        query_state->state = USE;

        // Sub-commands
        if (cmds[1] != NULL) {
            // Help: list all support sub commands
            if (strncmp(cmds[1], "-h", 2) == 0 ||
                strncmp(cmds[1], "help", 4) == 0) {
                use_command_info();
            }
            else {
                char db_folder[PATH_MAX] = {0};
                char db_filename[PATH_MAX] = {0};
                snprintf(db_folder, PATH_MAX, "%s/%s", WORKSPACE_PATH_FULL, cmds[1]);
                snprintf(db_filename, PATH_MAX, "%s/%s.json", db_folder, cmds[1]);

                if (!exist_file(db_filename)) {
                    fprintf(stderr, "Database %s not exist \n", cmds[1]);
                }
                else {
                    update_current_db(cmds[1], db_filename, db_folder);
                    current_db_t *db = get_current_db();
                    fprintf(stdout, "Using database: %s \n", db->name);
                }
            }
        }
    }
    // Select
    else if (strncmp(cmds[0], "select", 6) == 0) {
        query_state->state = SELECT;
        // Sub-commands
        if (cmds[1] != NULL) {
            // Help: list all support sub commands
            if (strncmp(cmds[1], "-h", 2) == 0 ||
                strncmp(cmds[1], "help", 4) == 0) {
                select_command_info();
            }
            // select all column
            // `select * from table_name`
            else if (strncmp(cmds[2], "from", 4) == 0) {
                char table_name_path[PATH_MAX] = {0};
                char *table_name = cmds[3];
                delete_semicolon(table_name);
                // Process `from` command
                if (check_table_exist((const char*)table_name, table_name_path)) {
                    // TODO: this should do several checks
                    // TODO: 1. `from` table exist
                    // TODO: 2. `select` column_names exist

                    // TODO: Steps:
                    // TODO: 1. get database meta data, read the table's column_names
                    // TODO: 2. parse SQL commands, into {"select": ["column_names"], "from": ["tables"], "where": ["args]}
                    // TODO: 3. Check parsed["select"] in meta["table"]["column_names"]
                    // TODO: 4. parse `where` args, to calc, ex: `age < 29`


                    // Load Table metadata
                    table_data_t *table_data = select_load_table_metadata(table_name);

                    // `select name,height from my_table where age<29

                    // Parse SQL command
                    size_t parsed_cmd_cnt = 0;
                    parsed_sql_cmd_t *parsed_cmd = parse_sql_cmd((const char *) prompt_buf->buf, &parsed_cmd_cnt);

                    // Process `select` command
                    bool column_found = select_fetch_available_column(table_data, &parsed_cmd[0]);

                    // Process `where` command
                    size_t condition_len = 0;
                    where_args_cond_t conditions[WHERE_MATCH_CNT] = {0};
                    bool row_status = select_fetch_available_row(table_data, &parsed_cmd[2], conditions, &condition_len);

                    // Make sure there is no non-exist table column
                    if  (column_found && row_status) {
                        current_db_t *db = get_current_db();
                        snprintf(table_name_path, PATH_MAX, "%s/%s.csv", db->folder_path, table_name);
                        select_load_table_data(table_data, table_name_path, conditions, condition_len);

                        select_table_display(table_data);
                        select_table_close(table_data);
                    }
                    else {
                        fprintf(stderr, "Column name not found \n");
                    }

                    // Free
                    parse_select_cmd_close(parsed_cmd);
                }
                else {
                    fprintf(stderr, "Table %s not found\n", cmds[3]);
                }
            }
        }
    }
    // Insert
    else if (strncmp(cmds[0], "insert", 6) == 0) {
        query_state->state = INSERT;

        // Sub-commands
        if (cmds[1] != NULL) {
            // Help: list all support sub commands
            if (strncmp(cmds[1], "-h", 2) == 0 ||
                strncmp(cmds[1], "help", 4) == 0) {
                insert_command_info();
            }
            else {
                char table_name_path[PATH_MAX] = {0};
                if (check_table_exist((const char*)cmds[1], table_name_path)) {
                    insert_table_data(table_name_path, (const char*)cmds[1], &cmds[3], num_tokens-3);
                }
                else {
                    fprintf(stderr, "Table %s not found\n", cmds[1]);
                }
            }
        }
    }
    // TODO: Add list command, like
    // TODO: `list database <database_name>` to show all database
    // TODO: `list table <table_name>` to show all table under current using database

    // TODO: Add info to show columns, like
    // TODO: info database <database_name> to print whole json file
    // TODO info table <table_name> only print specific table column_name

    // Undefined command
    else {
        query_state->state = UNDEFINED;
        fprintf(stderr, "Unrecognized command '%s' \n\n", prompt_buf->buf);
        basic_command_info();
    }

    splitter.free(cmds, num_tokens);
}

const char* query_state_to_string(State_t state) {
    switch (state) {
        case INIT: return "INIT";
        case HELP: return "HELP";
        case EXIT: return "EXIT";
        case CREATE: return "CREATE";
        case USE: return "USE";
        case DELETE: return "DELETE";
        case SELECT: return "SELECT";
        case INSERT: return "INSERT";
        case UNDEFINED: return "UNDEFINED";
        default: return "Unknown State";
    }
}

static void query_state_init(query_state_t *q) {
    q->state = INIT;
    q->args = NULL;
}

static void query_state_close(query_state_t *q) {
    if (q->args != NULL) {
        free(q->args);
        q->args = NULL;
    }

    if (q != NULL) {
        free(q);
        q = NULL;
    }
}

query_state_t* query_state_construct() {
    query_state_t *q = (query_state_t*)malloc(sizeof(query_state_t));
    if (q == NULL) {
        DB_ASSERT(!"Failed to allocate memory.\n");
    }
    memset(q, 0, sizeof(query_state_t));

    q->init = query_state_init;
    q->close = query_state_close;
    return q;
}
