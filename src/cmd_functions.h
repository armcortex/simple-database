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
#include <cJSON.h>


typedef enum {
    TABLE_STRING = 0,
    TABLE_INT,
    TABLE_FLOAT,
} table_data_enum_t;

typedef struct table_row_t {
    struct table_row_t *next;
    char **data;
} table_row_t;

typedef struct table_data_t {
    table_data_enum_t *types;
    char **table_column_names;
    table_row_t *rows;
    size_t len;
} table_data_t;


table_data_t* table_data_init(size_t len);
void table_data_close(table_data_t *t);
void table_data_add_type(table_data_t *t, const char *type, size_t idx);
void table_data_add_column_name(table_data_t *t, const char *column_name, size_t idx);
table_row_t* table_data_create_row_node(char **data, size_t len);
void table_data_insert_row_data(table_data_t *t, char **data);

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
table_data_t* select_table(const char *table_name, char *table_name_path, const char *select_columns,
                            const char **args, size_t args_len);

table_data_t* select_init_table_struct(const char *table_name);
void select_close_table_struct(void *ptr);
void select_table_display(table_data_t *table_data);
void select_table_close(table_data_t *table_data);

// MISC
const char* create_filename(const char *name, const char *ext);
const char* create_filename_full_path(const char *base, const char *name, const char *ext);
const char* str_concat(const char *format, ...);

bool check_table_exist(const char *table_name, char *table_name_path);

// TODO: add serialization() and deserialization() for table data


#ifdef __cplusplus
}
#endif

#endif //SIMPLE_DATABASE_CMD_FUNCTIONS_H
