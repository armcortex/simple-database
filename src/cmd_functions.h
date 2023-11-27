//
// Created by MCS51_M2 on 2023/11/22.
//

#ifndef SIMPLE_DATABASE_CMD_FUNCTIONS_H
#define SIMPLE_DATABASE_CMD_FUNCTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <cJSON.h>

// Help info
void basic_command_info(void);
void create_command_info(void);
void delete_command_info(void);
void use_command_info(void);
void insert_command_info(void);

// Create database command
void create_database(const char *name);
void create_database_meta(const char *name);
void add_database_new_table(const char *db_filename, cJSON *new_table);

// Create Table command
void create_table(const char *filename_path, const char *filename, char **args, size_t len);
cJSON *create_table_json(const char *name, char **args, size_t len);

// Insert command
void insert_table_data(const char *filename, char **datas, size_t len);

// Delete command
void delete_database(const char *name);
void delete_table(const char *name);
void delete_table_all(const char *db_base_path);

// MISC
const char* create_filename(const char *name, const char *ext);
const char* create_filename_full_path(const char *base, const char *name, const char *ext);
const char* str_concat(const char *format, ...);

// TODO: add serialization() and deserialization() for table data


#ifdef __cplusplus
}
#endif

#endif //SIMPLE_DATABASE_CMD_FUNCTIONS_H
