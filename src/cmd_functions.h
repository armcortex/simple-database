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
#include "table.h"
#include "operator.h"


// Table data manipulate
table_data_t *table_data_init(size_t col_len, size_t row_len);
void table_data_close(table_data_t *t);
void table_data_add_type(table_data_t *t, const char *type, size_t idx);
void table_data_add_column_name(table_data_t *t, const char *column_name, size_t idx);
table_row_t* table_data_create_row_node(char **data, size_t len);
void table_data_insert_row_data(table_data_t *t, char **data, size_t data_len);

// Help info
bool basic_fn(char **args, size_t args_len);
bool basic_sub_help(char **args, size_t args_len);
bool basic_sub_a_fn(char **args, size_t args_len);
bool basic_sub_b_fn(char **args, size_t args_len);

void basic_command_info(void);
void create_command_info(void);
void delete_command_info(void);
void use_command_info(void);
void insert_command_info(void);
void select_command_info(void);

void list_command_info(void);

// Create database command
bool create_database_help_fn(char **args, size_t args_len);
bool create_database_fn(char **args, size_t args_len);
void create_database(const char *name);
void create_database_meta(const char *name);
void add_database_new_table(const char *db_filename, cJSON *new_table);

// Create Table command
bool create_table_fn(char **args, size_t args_len);
void create_table(const char *filename_path, const char *filename, char **args, size_t len);
cJSON *create_table_json(const char *name, char **args, size_t len);

// Use Command
bool use_help_fn(char **args, size_t args_len);
bool use_fn(char **args, size_t args_len);

// Insert command
bool insert_help_fn(char **args, size_t args_len);
bool insert_fn(char **args, size_t args_len);
void insert_table_data(const char *filename_path, const char *table_name, char **datas, size_t len);
void insert_table_update_database_meta(const char *db_filename, const char *table_name, size_t data_update_cnt);

// Delete command
bool delete_help_fn(char **args, size_t args_len);
bool delete_database_fn(char **args, size_t args_len);
void delete_database(const char *name);
void delete_table(const char *name);
void delete_table_all(const char *db_base_path);

// Select command
bool select_help_fn(char **args, size_t args_len);
bool select_fn(char **args, size_t args_len);
void select_load_table_data(table_data_t *t, char *table_name_path, where_args_cond_t *conditions, size_t condition_len);
table_data_t* select_load_table_metadata(const char *table_name);
bool select_fetch_available_column(table_data_t *t, parsed_sql_cmd_t *select_cmd);
void select_parse_where_args(table_data_t *t, const char *sql_cmd, where_args_cond_t *conds, size_t *args_len);
bool select_fetch_available_row(table_data_t *t, parsed_sql_cmd_t *select_cmd, where_args_cond_t *conditions, size_t *condition_len);
void select_table_display(table_data_t *t);
void select_table_close(table_data_t *t);

// List command

// MISC
bool check_table_exist(const char *table_name, char *table_name_path);
bool compare_column_name(const char *ref, const char *src);
size_t find_column_name_idx(table_data_t *t, const char *col_name);

bool wrong_fn(char **args, size_t args_len);
bool null_fn(char **args, size_t args_len);
bool undefined_fn(char **args, size_t args_len);
bool exit_fn(char **args, size_t args_len);


#ifdef __cplusplus
}
#endif

#endif //SIMPLE_DATABASE_CMD_FUNCTIONS_H
