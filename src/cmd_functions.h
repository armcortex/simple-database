//
// Created by MCS51_M2 on 2023/11/22.
//

#ifndef SIMPLE_DATABASE_CMD_FUNCTIONS_H
#define SIMPLE_DATABASE_CMD_FUNCTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <cJSON.h>

#include "db_config.h"

#define WHERE_MATCH_CNT     (20)

typedef enum {
    TABLE_STRING = 0,
    TABLE_INT,
    TABLE_FLOAT,
} table_col_type_t;

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


// Table data manipulate
table_data_t *table_data_init(size_t col_len, size_t row_len);
void table_data_close(table_data_t *t);
void table_data_add_type(table_data_t *t, const char *type, size_t idx);
void table_data_add_column_name(table_data_t *t, const char *column_name, size_t idx);
table_row_t* table_data_create_row_node(char **data, size_t len);
void table_data_insert_row_data(table_data_t *t, char **data, size_t data_len);

// Help info
void basic_command_info(void);
void create_command_info(void);
void delete_command_info(void);
void use_command_info(void);
void insert_command_info(void);
void select_command_info(void);

// Create database command
void create_database(const char *name);
void create_database_meta(const char *name);
void add_database_new_table(const char *db_filename, cJSON *new_table);

// Create Table command
void create_table(const char *filename_path, const char *filename, char **args, size_t len);
cJSON *create_table_json(const char *name, char **args, size_t len);

// Insert command
void insert_table_data(const char *filename_path, const char *table_name, char **datas, size_t len);
void insert_table_update_database_meta(const char *db_filename, const char *table_name, size_t data_update_cnt);

// Delete command
void delete_database(const char *name);
void delete_table(const char *name);
void delete_table_all(const char *db_base_path);

// Select command
void select_load_table_data(table_data_t *t, char *table_name_path, where_args_cond_t *conditions, size_t condition_len);

table_data_t* select_load_table_metadata(const char *table_name);
bool select_fetch_available_column(table_data_t *t, parsed_sql_cmd_t *select_cmd);

void parse_where_args(table_data_t *t, const char *sql_cmd, where_args_cond_t *conds, size_t *args_len);
bool is_op_and_or(logic_op_t op);
bool is_op_parenthesis(logic_op_t op);
bool is_operator(logic_op_t op);
bool is_operand(logic_op_t op);

bool select_fetch_available_row(table_data_t *t, parsed_sql_cmd_t *select_cmd, where_args_cond_t *conditions, size_t *condition_len);
void select_table_display(table_data_t *t);
void select_table_close(table_data_t *t);

// MISC
const char* create_filename(const char *name, const char *ext);
const char* create_filename_full_path(const char *base, const char *name, const char *ext);
const char* str_concat(const char *format, ...);

bool check_table_exist(const char *table_name, char *table_name_path);
bool compare_column_name(const char *ref, const char *src);
size_t find_column_name_idx(table_data_t *t, const char *col_name);


// TODO: add serialization() and deserialization() for table data


#ifdef __cplusplus
}
#endif

#endif //SIMPLE_DATABASE_CMD_FUNCTIONS_H
