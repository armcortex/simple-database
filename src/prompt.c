//
// Created by MCS51_M2 on 2023/11/16.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prompt.h"

void print_prompt() {
    printf("> ");
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

void check_command(prompt_buf_t *prompt_buf) {
    // Exit
    if (strncmp(prompt_buf->buf, "exit", 4) == 0) {
        printf("Bye Bye \n");
        free_prompt_buf(prompt_buf);
        exit(EXIT_SUCCESS);
    }
    // Undefined command
    else {
        printf("Unrecognized command '%s' \n", prompt_buf->buf);
    }
}