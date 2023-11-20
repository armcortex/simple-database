//
// Created by MCS51_M2 on 2023/11/16.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prompt.h"
#include "helper_functions.h"

void print_prompt() {
    fprintf(stdout, "> ");
}

prompt_buf_t* new_prompt_buf() {
    prompt_buf_t *prompt_buf = (prompt_buf_t*)malloc(sizeof(prompt_buf_t));
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

void check_commands(prompt_buf_t *prompt_buf) {
    Splitter_t splitter = split_construct();
    size_t num_tokens;
    char** cmds = splitter.run(prompt_buf->buf, " ", &num_tokens);

    // select * from db

    // Exit
    if (strncmp(cmds[0], "exit", 4) == 0) {
        fprintf(stdout, "Bye Bye \n");
        free_prompt_buf(prompt_buf);
        exit(EXIT_SUCCESS);
    }
    // Undefined command
    else {
        fprintf(stderr, "Unrecognized command '%s' \n", prompt_buf->buf);
    }
}