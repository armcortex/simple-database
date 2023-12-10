//
// Created by MCS51_M2 on 2023/12/8.
//

#ifndef SIMPLE_DATABASE_PARSER_H
#define SIMPLE_DATABASE_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MATCHES             (4)
#define MAX_SELECT_CMD_LEN      (128)

typedef enum {
    SELECT_SELECT_CMD = 0,
    SELECT_FROM_CMD,
    SELECT_WHERE_CMD,
} select_state_t;

typedef struct select_parsed_data_t {
    select_state_t state;
    void **args;
} select_parsed_data_t;




void parse_select_cmd(const char *sql_cmd, select_parsed_data_t *parsed_data);


#ifdef __cplusplus
}
#endif

#endif //SIMPLE_DATABASE_PARSER_H
