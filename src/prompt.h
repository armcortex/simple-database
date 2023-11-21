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

typedef enum {
    INIT = 0,
    EXIT,
    CREATE,
    USE,
    DROP,
    SELECT,
    UNDEFINED,
    COUNT,
}State_t;

typedef struct {
    State_t state;
} query_state_t;

void print_prompt(void);
prompt_buf_t* new_prompt_buf(void);
void free_prompt_buf(prompt_buf_t *prompt_buf);

void check_commands(prompt_buf_t *prompt_buf, query_state_t *query_state);

#ifdef __cplusplus
}
#endif

#endif //SIMPLE_DATABASE_PROMPT_H
