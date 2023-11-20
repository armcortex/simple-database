//
// Created by MCS51_M2 on 2023/11/16.
//

#ifndef SIMPLE_DATABASE_PROMPT_H
#define SIMPLE_DATABASE_PROMPT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

typedef struct {
    char *buf;
    size_t len;
} prompt_buf_t;

void print_prompt(void);
prompt_buf_t* new_prompt_buf(void);
void free_prompt_buf(prompt_buf_t *prompt_buf);

void check_commands(prompt_buf_t *prompt_buf);

#ifdef __cplusplus
}
#endif

#endif //SIMPLE_DATABASE_PROMPT_H
