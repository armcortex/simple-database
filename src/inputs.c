//
// Created by MCS51_M2 on 2023/11/19.
//

#include "inputs.h"

void read_input(prompt_buf_t *prompt_buf) {
    if (!prompt_buf) {
        fprintf(stderr, "prompt_buf is null");
    }

    ssize_t read_bytes = getline(&(prompt_buf->buf), &(prompt_buf->len), stdin);
    if (read_bytes <= 0) {
        fprintf(stderr, "Reading Error \n");
        free_prompt_buf(prompt_buf);
//        exit(EXIT_FAILURE);
    }

    // Delete '\n' newline
    prompt_buf->len = read_bytes - 1;
    prompt_buf->buf[read_bytes-1] = 0;

    if (VERBOSE) {
        fprintf(stderr, "Read %zu bytes, Input: %s \n", prompt_buf->len, prompt_buf->buf);
    }
}