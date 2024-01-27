//
// Created by MCS51_M2 on 2023/11/16.
//

#ifndef SIMPLE_DATABASE_PROMPT_H
#define SIMPLE_DATABASE_PROMPT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "db_config.h"

typedef enum {
    INIT = 0,
    HELP,
    EXIT,
    CREATE,
    USE,
    DELETE,
    SELECT,
    INSERT,
    LIST,
    UNDEFINED,
    COUNT,
} State_t;

typedef struct {
    char *buf;
    size_t len;
} prompt_buf_t;

typedef struct query_state_t {
    State_t state;
    char **args;
    size_t args_len;
    void (*init)(struct query_state_t *query_state);
    void (*close)(struct query_state_t *query_state);
} query_state_t;

typedef struct cmd_fn_t {
    State_t state;
    bool (*callback_fn)(char **args, size_t args_len);
} cmd_fn_t;


void print_prompt(void);
prompt_buf_t* new_prompt_buf(void);
void free_prompt_buf(prompt_buf_t *prompt_buf);

const char* query_state_to_string(State_t state);
void parse_commands(prompt_buf_t *prompt_buf, query_state_t *query_state);
void execute_commands(query_state_t *query_state);

query_state_t* query_state_construct(void);

#ifdef __cplusplus
}
#endif

#endif //SIMPLE_DATABASE_PROMPT_H
