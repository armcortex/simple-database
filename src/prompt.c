//
// Created by MCS51_M2 on 2023/11/16.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "prompt.h"
#include "helper_functions.h"
#include "cmd_functions.h"
#include "database.h"

void print_prompt() {
    current_db_t *db = get_current_db();
    fprintf(stdout, "%s > ", db->name);
}

prompt_buf_t* new_prompt_buf() {
    prompt_buf_t *prompt_buf = (prompt_buf_t*)malloc(sizeof(prompt_buf_t));
    if (prompt_buf == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
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
                const char *db_filename = str_concat("%s/%s/%s.txt", WORKSPACE_PATH_FULL, cmds[2], cmds[2]);
                create_database(db_filename);
                fprintf(stdout, "Create database at: %s \n", db_filename);

                // Create meta data
                create_database_meta(db_filename);
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
                const char *db_filename = str_concat("%s/%s/%s.txt", WORKSPACE_PATH_FULL, cmds[2], cmds[2]);
                delete_database(db_filename);

                // Delete folder
                const char *db_folder_name = str_concat("%s/%s", WORKSPACE_PATH_FULL, cmds[2]);
                remove_folder(db_folder_name);

                fprintf(stdout, "Delete database at: %s \n", db_filename);
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
                const char *db_filename = str_concat("%s/%s/%s.txt", WORKSPACE_PATH_FULL, cmds[1], cmds[1]);
                if (!exist_file(db_filename)) {
                    fprintf(stderr, "Database %s not exist \n", cmds[1]);
                }
                else {
                    update_current_db(cmds[1]);
                    current_db_t *db = get_current_db();
                    fprintf(stdout, "Using database: %s \n", db->name);
                }
            }
        }
    }
    // Select
    else if (strncmp(cmds[0], "select", 6) == 0) {
        query_state->state = SELECT;
    }
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
    }

    if (q != NULL) {
        free(q);
    }
}

query_state_t* query_state_construct() {
    query_state_t *q = (query_state_t*)malloc(sizeof(query_state_t));
    if (q == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        assert(q != NULL);
    }
    memset(q, 0, sizeof(query_state_t));

    q->init = query_state_init;
    q->close = query_state_close;
    return q;
}
