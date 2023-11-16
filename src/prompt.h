//
// Created by MCS51_M2 on 2023/11/16.
//

#ifndef SIMPLE_DATABASE_PROMPT_H
#define SIMPLE_DATABASE_PROMPT_H

#include <stdio.h>

typedef struct {
    char *buf;
    size_t len;
} prompt_buf_t;

void print_prompt(void);
prompt_buf_t* new_prompt_buf(void);
void free_prompt_buf(prompt_buf_t *prompt_buf);

#endif //SIMPLE_DATABASE_PROMPT_H
