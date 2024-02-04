//
// Created by MCS51_M2 on 2023/11/16.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "prompt.h"
#include "helper_functions.h"
#include "cmd_functions.h"
#include "database.h"
#include "parser.h"
#include "table.h"

// Parse function pointer
static cmd_parse_t parse_help_cmd_list[] = {
    {"-help", 5, HELP_SUB_HELP, NULL, 0},
    {"-a_fn", 5, HELP_SUB_A, NULL, 0},
    {"-b_fn", 5, HELP_SUB_B, NULL, 0},
};

static cmd_parse_t parse_create_cmd_list[] = {
    {"-help", 5, CREATE_SUB_HELP, NULL, 0},
    {"-h", 2, CREATE_SUB_HELP, NULL, 0},
    {"-database", 9, CREATE_SUB_DATABASE, NULL, 0},
    {"-table", 6, CREATE_SUB_TABLE, NULL, 0},
};

static cmd_parse_t parse_use_cmd_list[] = {
    {"-help", 5, USE_SUB_HELP, NULL, 0},
    {"-h", 2, USE_SUB_HELP, NULL, 0},
};

static cmd_parse_t parse_delete_cmd_list[] = {
    {"-help", 5, DELETE_SUB_HELP, NULL, 0},
    {"-h", 2, DELETE_SUB_HELP, NULL, 0},
    {"-database", 9, DELETE_SUB_DATABASE, NULL, 0},
};

static cmd_parse_t parse_insert_cmd_list[] = {
        {"-help", 5, INSERT_SUB_HELP, NULL, 0},
        {"-h", 2, INSERT_SUB_HELP, NULL, 0},
};

static cmd_parse_t parse_basic_cmd_list[] = {
    {"help", 4, HELP, parse_help_cmd_list, CALC_ARRAY_SIZE(parse_help_cmd_list, cmd_parse_t)},
    {"exit", 4, EXIT, NULL, 0},
    {"create", 6, CREATE, parse_create_cmd_list, CALC_ARRAY_SIZE(parse_create_cmd_list, cmd_parse_t)},
    {"use", 3, USE, parse_use_cmd_list, CALC_ARRAY_SIZE(parse_use_cmd_list, cmd_parse_t)},
    {"delete", 6, DELETE, parse_delete_cmd_list, CALC_ARRAY_SIZE(parse_delete_cmd_list, cmd_parse_t)},
    {"select", 6, SELECT, NULL, 0},
    {"insert", 6, INSERT, parse_insert_cmd_list, CALC_ARRAY_SIZE(parse_insert_cmd_list, cmd_parse_t)},
    {"list", 4, LIST, NULL, 0},
};


// Execute function pointer
static cmd_fn_t help_subcmd_fn_list[] = {
    {HELP_SUB_HELP, basic_sub_help, NULL},
    {HELP_SUB_A, basic_sub_a_fn, NULL},
    {HELP_SUB_B, basic_sub_b_fn, NULL},
};

static cmd_fn_t create_subcmd_fn_list[] = {
    {CREATE_SUB_HELP, create_database_help_fn, NULL},
    {CREATE_SUB_DATABASE, create_database_fn, NULL},
    {CREATE_SUB_TABLE, create_table_fn, NULL},
};

static cmd_fn_t use_subcmd_fn_list[] = {
    {USE_SUB_HELP, use_help_fn, NULL},
};

static cmd_fn_t delete_subcmd_fn_list[] = {
    {DELETE_SUB_HELP, delete_help_fn, NULL},
    {DELETE_SUB_DATABASE, delete_database_fn, NULL},
};

static cmd_fn_t insert_subcmd_fn_list[] = {
        {INSERT_SUB_HELP, insert_help_fn, NULL},
};

// Data order must be same as cmd_state_t
static cmd_fn_t main_cmd_fn_list[] = {
    {INIT, null_fn, NULL},
    {HELP, basic_fn, help_subcmd_fn_list},
    {EXIT, NULL, NULL},
    {CREATE, wrong_fn, create_subcmd_fn_list},
    {USE, use_fn, use_subcmd_fn_list},
    {DELETE, wrong_fn, delete_subcmd_fn_list},
    {SELECT, null_fn, NULL},
    {INSERT, insert_fn, insert_subcmd_fn_list},
    {LIST, null_fn, NULL},
};

void print_prompt() {
    const current_db_t *db = get_current_db();
    fprintf(stdout, "%s > ", db->name);
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

void parse_commands(prompt_buf_t *prompt_buf, query_state_t *query_state) {
    splitter_t splitter = split_construct();
    size_t cmds_len;
    char** cmds = splitter.run(prompt_buf->buf, " ", &cmds_len);

    // get `enter` only
    if (cmds_len == 0) {
        query_state->state = HELP;
        splitter.free(cmds, cmds_len);
        return;
    }
    else {
        // parse args
        query_state->args_len = cmds_len - 1;
        query_state->args = (char**)calloc(query_state->args_len, sizeof(char*));
        for (size_t i=0; i<query_state->args_len; i++) {
            query_state->args[i] = strdup(cmds[i+1]);
        }
    }

    // calc basic command state
    // TODO: refactor here, can be better
    query_state->state = UNDEFINED;
    size_t cmd_cnt = CALC_ARRAY_SIZE(parse_basic_cmd_list, cmd_parse_t);
    for (size_t i=0; i<cmd_cnt; i++) {
        cmd_parse_t cmd_tmp = parse_basic_cmd_list[i];
        if ((cmds_len > 0) && (strncmp(cmds[0], cmd_tmp.s, cmd_tmp.s_len) == 0)) {
            query_state->state = cmd_tmp.state;

            // calc sub-command state
            if (cmd_tmp.sub_parse != NULL) {
                cmd_parse_t *sub_fns = cmd_tmp.sub_parse;
                query_state->sub_state = sub_fns[0].state;
                if (cmds_len == 1) break;
                else {
                    for (size_t j=0; j<cmd_tmp.sub_fn_cnt; j++) {
                        // Check this is sub-command or not
                        if (strncmp(cmds[1], "-", 1) != 0) {
                            query_state->sub_state = RUN_MAIN_CMD;
                            break;
                        }
                        if (strncmp(cmds[1], sub_fns[j].s, sub_fns[j].s_len) == 0) {
                            query_state->sub_state = sub_fns[j].state;
                            break;
                        }
                    }
                }
            }
            break;
        }
    }

    // Free
    splitter.free(cmds, cmds_len);
}

void execute_commands(query_state_t *query_state) {
    if (main_cmd_fn_list[query_state->state].state != query_state->state) {
        DB_ASSERT(!"Command states not matched\n");
    }

    cmd_fn_t *sub_fn = main_cmd_fn_list[query_state->state].sub_fn;
    if ((sub_fn) && (query_state->sub_state != RUN_MAIN_CMD)) {
        size_t fn_offset = query_state->sub_state - sub_fn[0].state;
        sub_fn[fn_offset].callback_fn(query_state->args, query_state->args_len);
    }
    else {
        main_cmd_fn_list[query_state->state].callback_fn(query_state->args, query_state->args_len);
    }
}

#if 0
void check_commands_del(prompt_buf_t *prompt_buf, query_state_t *query_state) {
    splitter_t splitter = split_construct();
    size_t num_tokens;
    char** cmds = splitter.run(prompt_buf->buf, " ", &num_tokens);

    // `enter` only
    if (num_tokens == 0) {
        query_state->state = HELP;
        basic_command_info();
        splitter.free(cmds, num_tokens);
        return;
    }

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
                char db_foldername_full[PATH_MAX] = {0};
                snprintf(db_foldername_full, PATH_MAX, "%s/%s", WORKSPACE_PATH_FULL, cmds[2]);
                create_folder(db_foldername_full);

                // Create database file
                char db_filename_full[PATH_MAX] = {0};
                snprintf(db_filename_full, PATH_MAX, "%s/%s/%s.json", WORKSPACE_PATH_FULL, cmds[2], cmds[2]);
                create_database(db_filename_full);
                fprintf(stdout, "Create database at: %s \n", db_filename_full);
            }
            else if (strncmp(cmds[1], "table", 5) == 0) {
                const current_db_t *curr_db =  get_current_db();
                if (curr_db->len == 0) {
                    fprintf(stderr, "Don't know what database to use, please use `USE` command to select database first \n");
                }
                // Make sure command contain `create table table_name` and `column_name + type` pair
                else if ((num_tokens < 5) || ((num_tokens-3) % 2 != 0)) {
                    fprintf(stderr, "argument not enough: (<column name> <column type> ...)");
                }
                else {
                    char table_filename_full[PATH_MAX] = {0};
                    snprintf(table_filename_full, PATH_MAX, "%s/%s/%s.csv", WORKSPACE_PATH_FULL, curr_db->name, cmds[2]);
                    create_table(table_filename_full, cmds[2], &cmds[3], num_tokens-3);
                    fprintf(stdout, "Create table at: %s \n", table_filename_full);
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
                char db_filename_full[PATH_MAX] = {0};
                snprintf(db_filename_full, PATH_MAX, "%s/%s/%s.json", WORKSPACE_PATH_FULL, cmds[2], cmds[2]);
                delete_database(db_filename_full);

                // Delete folder
                char db_foldername_full[PATH_MAX] = {0};
                snprintf(db_foldername_full, PATH_MAX, "%s/%s", WORKSPACE_PATH_FULL, cmds[2]);

                delete_table_all(db_foldername_full);
                remove_folder(db_foldername_full);

                // clean buffer
                clean_current_db();
            }
            // TODO: implement "delete table" sub-command
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
                // use_command_info();
                ;
            }
            else {
                char db_folder_full[PATH_MAX] = {0};
                char db_filename_full[PATH_MAX] = {0};
                snprintf(db_folder_full, PATH_MAX, "%s/%s", WORKSPACE_PATH_FULL, cmds[1]);
                snprintf(db_filename_full, PATH_MAX, "%s/%s.json", db_folder_full, cmds[1]);

                if (!exist_file(db_filename_full)) {
                    fprintf(stderr, "Database %s not exist \n", cmds[1]);
                }
                else {
                    update_current_db(cmds[1], db_filename_full, db_folder_full);
                    const current_db_t *db = get_current_db();
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
                // select_command_info();
                ;
            }
            // select all column
            // `select * from table_name`
            else if (strncmp(cmds[2], "from", 4) == 0) {
                char table_name_path[PATH_MAX] = {0};
                char *table_name = cmds[3];
                delete_semicolon(table_name);
                // Process `from` command
                if (check_table_exist((const char*)table_name, table_name_path)) {
                    // Load Table metadata
                    table_data_t *table_data = select_load_table_metadata(table_name);

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
                        const current_db_t *db = get_current_db();
                        snprintf(table_name_path, PATH_MAX, "%s/%s.csv", db->folder_path, table_name);
                        select_load_table_data(table_data, table_name_path, conditions, condition_len);

                        select_table_display(table_data);
                    }
                    else {
                        fprintf(stderr, "Column name not found \n");
                    }

                    // Free
                    select_table_close(table_data);
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
                // insert_command_info();
                ;
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
    // List
    else if (strncmp(cmds[0], "list", 6) == 0) {
        query_state->state = LIST;

        // Sub-commands
        if (cmds[1] != NULL) {
            // Help: list all support sub commands
            if (strncmp(cmds[1], "-h", 2) == 0 ||
                strncmp(cmds[1], "help", 4) == 0) {
                list_command_info();
            }
            // else if (strncmp(cmds[1], "database", 8) == 0) {
            //     ;
            // }
            else {
                fprintf(stderr, "Unrecognized command '%s' \n\n", prompt_buf->buf);
            }
        }

    }




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
#endif

const char* query_state_to_string(cmd_state_t state) {
    switch (state) {
        case INIT: return "INIT";
        case HELP: return "HELP";
        case EXIT: return "EXIT";
        case CREATE: return "CREATE";
        case USE: return "USE";
        case DELETE: return "DELETE";
        case SELECT: return "SELECT";
        case INSERT: return "INSERT";
        case LIST: return "LIST";
        case UNDEFINED: return "UNDEFINED";
        default: return "Unknown State";
    }
}

static void query_state_init(query_state_t *q) {
    q->state = INIT;
    q->sub_state = INIT;
    q->args = NULL;
    q->args_len = 0;
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
