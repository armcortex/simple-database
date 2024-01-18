//
// Created by MCS51_M2 on 2024/1/18.
//

#ifndef SIMPLE_DATABASE_TABLE_H
#define SIMPLE_DATABASE_TABLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

#include "db_config.h"

#define WHERE_MATCH_CNT     (20)

typedef enum {
    TABLE_STRING = 0,
    TABLE_INT,
    TABLE_FLOAT,
} table_col_type_t;

typedef enum {
    OP_NULL = 0,
    OP_AND,                 // &&
    OP_OR,                  // ||
    OP_EQ,                  // =
    OP_NE,                  // !=
    OP_LT,                  // <
    OP_GT,                  // >
    OP_LE,                  // <=
    OP_GE,                  // >=
    OP_OPEN_PARENTHESIS,    // (
    OP_CLOSE_PARENTHESIS,   // )
} logic_op_t;

typedef struct row_cell_t {
    char cell[CELL_TEXT_MAX];
} row_cell_t;

typedef struct table_row_t {
    row_cell_t *data;
    uint8_t enable;
    struct table_row_t *next;
} table_row_t;

typedef struct table_col_t {
    char name[CELL_TEXT_MAX];
    table_col_type_t type;
    uint8_t enable;
} table_col_t;

typedef struct table_data_t {
    table_col_t *cols;
    table_row_t *rows;
    size_t col_len;
    size_t col_enable_cnt;
    size_t row_len;
} table_data_t;

typedef enum {
    SQL_SELECT_CMD = 0,
    SQL_FROM_CMD,
    SQL_WHERE_CMD,
} sql_state_t;

typedef struct parsed_sql_cmd_t {
    sql_state_t state;
    char *args;
} parsed_sql_cmd_t;

typedef struct value_type_t {
    char s[CELL_TEXT_MAX];
    int i;
    float f;
} value_type_t;

typedef struct where_args_cond_t {
    char column[CELL_TEXT_MAX];
    logic_op_t op;
    value_type_t val;
} where_args_cond_t;



#ifdef __cplusplus
}
#endif

#endif //SIMPLE_DATABASE_TABLE_H
