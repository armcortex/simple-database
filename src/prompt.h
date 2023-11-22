//
// Created by MCS51_M2 on 2023/11/16.
//

#ifndef SIMPLE_DATABASE_PROMPT_H
#define SIMPLE_DATABASE_PROMPT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

typedef enum {
    INIT = 0,
    HELP,
    EXIT,
    CREATE,
    USE,
    DROP,
    SELECT,
    UNDEFINED,
    COUNT,
}State_t;

typedef struct {
    char *buf;
    size_t len;
} prompt_buf_t;

typedef struct query_state_t {
    State_t state;
    char *args;
    void (*init)(struct query_state_t *query_state);
    void (*close)(struct query_state_t *query_state);
} query_state_t;

void print_prompt(void);
prompt_buf_t* new_prompt_buf(void);
void free_prompt_buf(prompt_buf_t *prompt_buf);

const char* query_state_to_string(State_t state);
void check_commands(prompt_buf_t *prompt_buf, query_state_t *query_state);

query_state_t* query_state_construct(void);

#ifdef __cplusplus
}
#endif

#endif //SIMPLE_DATABASE_PROMPT_H
