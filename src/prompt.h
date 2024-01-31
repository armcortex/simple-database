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
    RUN_MAIN_CMD,
    HELP_SUB_HELP,
    HELP_SUB_A,
    HELP_SUB_B,
    CREATE_SUB_HELP,
    CREATE_SUB_DATABASE,
    CREATE_SUB_TABLE,
    USE_SUB_HELP,
    COUNT,
} cmd_state_t;

typedef struct {
    char *buf;
    size_t len;
} prompt_buf_t;

typedef struct query_state_t {
    cmd_state_t state;
    cmd_state_t sub_state;
    char **args;
    size_t args_len;
    void (*init)(struct query_state_t *query_state);
    void (*close)(struct query_state_t *query_state);
} query_state_t;


typedef struct cmd_parse_t {
    const char *s;
    size_t s_len;
    cmd_state_t state;
    struct cmd_parse_t *sub_parse;
    size_t sub_fn_cnt;
} cmd_parse_t;

typedef bool (*cmd_fn_cb_t)(char **args, size_t args_len);

typedef struct cmd_fn_t {
    cmd_state_t state;
    cmd_fn_cb_t callback_fn;
    struct cmd_fn_t *sub_fn;
} cmd_fn_t;


void print_prompt(void);
prompt_buf_t* new_prompt_buf(void);
void free_prompt_buf(prompt_buf_t *prompt_buf);

const char* query_state_to_string(cmd_state_t state);
void parse_commands(prompt_buf_t *prompt_buf, query_state_t *query_state);
void execute_commands(query_state_t *query_state);

query_state_t* query_state_construct(void);

#ifdef __cplusplus
}
#endif

#endif //SIMPLE_DATABASE_PROMPT_H
