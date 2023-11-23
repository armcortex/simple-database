//
// Created by MCS51_M2 on 2023/11/16.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <limits.h>

#include "prompt.h"
#include "helper_functions.h"
#include "cmd_functions.h"
#include "db_config.h"

void print_prompt() {
    fprintf(stdout, "> ");
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

        // Help: list all support sub commands
        if (cmds[1] != NULL) {
            if (strncmp(cmds[1], "-h", 2) == 0) {
                create_command_info();
            }
            else if (strncmp(cmds[1], "database", 8) == 0) {
                check_current_path();
                const char *db_filename = create_filename_full_path(WORKSPACE_PATH_FULL, cmds[2], ".txt");
                fprintf(stdout, "filename: %s \n", db_filename);
                create_database(db_filename);
            }
            else {
                fprintf(stderr, "Unrecognized command '%s' \n\n", prompt_buf->buf);
            }
        }
    }
    // Delete
    else if (strncmp(cmds[0], "drop", 4) == 0) {
        query_state->state = DROP;
    }
    // Use database
    else if (strncmp(cmds[0], "use", 3) == 0) {
        query_state->state = USE;
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
}

const char* query_state_to_string(State_t state) {
    switch (state) {
        case INIT: return "INIT";
        case HELP: return "HELP";
        case EXIT: return "EXIT";
        case CREATE: return "CREATE";
        case USE: return "USE";
        case DROP: return "DROP";
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
