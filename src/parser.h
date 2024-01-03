//
// Created by MCS51_M2 on 2023/12/8.
//

#ifndef SIMPLE_DATABASE_PARSER_H
#define SIMPLE_DATABASE_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

#include "cmd_functions.h"

#define MAX_MATCHES             (5)
#define MAX_SELECT_CMD_LEN      (128)



parsed_sql_cmd_t* parse_sql_cmd(const char *sql_cmd, size_t *match_cnt);
void parse_select_cmd_close(parsed_sql_cmd_t *parsed_data);



#ifdef __cplusplus
}
#endif

#endif //SIMPLE_DATABASE_PARSER_H
