//
// Created by MCS51_M2 on 2023/11/22.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>
#include <stdarg.h>
#include <time.h>
#include <cJSON.h>

#include "cmd_functions.h"
#include "db_config.h"
#include "helper_functions.h"
#include "database.h"

static char g_db_file_path[PATH_MAX] = {0};
static char g_db_name[DB_NAME_MAX] = {0};

void basic_command_info() {
    fprintf(stdout, "All Support commands: \n");
    fprintf(stdout, "\t help: \n");
    fprintf(stdout, "\t exit: \n");
    fprintf(stdout, "\t create: \n");
    fprintf(stdout, "\t use: \n");
    fprintf(stdout, "\t drop: \n");
    fprintf(stdout, "\t select: \n");
}

void create_command_info() {
    fprintf(stdout, "Create sub-commands: \n");
    fprintf(stdout, "\t database <database name> \n");
    fprintf(stdout, "\t table <table name> (<column name> <column type> ...) \n");
}

void delete_command_info() {
    fprintf(stdout, "Delete sub-commands: \n");
    fprintf(stdout, "\t database <database name> \n");
    fprintf(stdout, "\t table <table name> \n");
}

void use_command_info() {
    fprintf(stdout, "Use sub-commands: \n");
    fprintf(stdout, "\t <database name> \n");
}

void create_database(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to create database: %s \n", filename);
        assert(0);
    }

    // Get filename, and write in the beginning of the file
    splitter_t splitter = split_construct();
    size_t num_tokens;
    char** filename_array = splitter.run(filename, "/", &num_tokens);
//    fprintf(file, "// Database: %s \n\n", filename_array[num_tokens-1]);
    splitter.free(filename_array, num_tokens);

    fclose(file);
}

void create_database_meta(const char *filename) {
    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        fprintf(stderr, "Failed to create JSON object \n");
        assert(0);
    }

    // Calc time
    char formatted_time[50] = {0};
    time_t current_time = time(NULL);
    strftime(formatted_time, sizeof(formatted_time), "%Y-%m-%d %H:%M:%S",
            localtime(&current_time));

    // Create json element
    cJSON_AddStringToObject(json, "time", formatted_time);
    cJSON_AddNumberToObject(json, "timestamp", (int)current_time);
    cJSON_AddNumberToObject(json, "table_cnt", 0);

    // Create an empty array
    cJSON *tables_array = cJSON_CreateArray();
    if (tables_array == NULL) {
        fprintf(stderr, "Failed to create tables array \n");
        cJSON_Delete(json);
        assert(0);
    }
    cJSON_AddItemToObject(json, "tables", tables_array);

    // Write to file
    char *json_str = cJSON_Print(json);
    if (json_str == NULL) {
        fprintf(stderr, "Failed to init cJSON_Print() \n");
        cJSON_Delete(json);
        assert(0);
    }

    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        fprintf(stderr, "Failed to write database: %s \n", filename);
        cJSON_free(json_str);
        cJSON_Delete(json);
        assert(0);
    }
    fprintf(file, "%s", json_str);
    fclose(file);
    cJSON_free(json_str);
    cJSON_Delete(json);
}

void add_database_new_table(const char *db_filename, cJSON *new_table) {
    // Open database file
    FILE *file = fopen(db_filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open database file: %s\n", db_filename);
        assert(0);
    }

    // Read the file content
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *content = (char*)malloc(length + 1);
    if (content == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        fclose(file);
        assert(0);
    }
    fread(content, 1, length, file);
    fclose(file);
    content[length] = '\0';

    // Parse the JSON content
    cJSON *json = cJSON_Parse(content);
    free(content);
    if (json == NULL) {
        fprintf(stderr, "Failed to parse JSON\n");
        assert(0);
    }

    // Get the tables array
    cJSON *tables = cJSON_GetObjectItem(json, "tables");
    if (!cJSON_IsArray(tables)) {
        fprintf(stderr, "\"table\" is not an array\n");
        cJSON_Delete(json);
        assert(0);
    }

    // Add the new table to the tables array
    cJSON_AddItemToArray(tables, new_table);

    // Update table_cnt
    cJSON *table_cnt = cJSON_GetObjectItem(json, "table_cnt");
    if (cJSON_IsNumber(table_cnt)) {
        table_cnt->valuedouble = cJSON_GetArraySize(tables); // Update count
    }

    // Write json back to the file
    file = fopen(db_filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to open database file:: %s\n", db_filename);
        cJSON_Delete(json);
        assert(0);
    }
    char *modified_content = cJSON_Print(json);
    fprintf(file, "%s", modified_content);
    fclose(file);
    cJSON_free(modified_content);
    cJSON_Delete(json);
}

void delete_database(const char *filename) {
    if (remove(filename) != 0) {
        fprintf(stderr, "Failed to delete database: %s \n", filename);
        assert(0);
    }
}

void create_table(const char *filename_path, const char *filename, char **args, size_t len) {
    FILE *file = fopen(filename_path, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to create table: %s \n", filename_path);
        assert(0);
    }

    // Show column on the first line
    for (size_t i=0; i<(len-2); i+=2) {
        fprintf(file, "%s, ", args[i]);
    }
    fprintf(file, "%s\n\n", args[len-2]);
    fclose(file);

    // Update new table to database meta data
    current_db_t *curr_db = get_current_db();
    cJSON *new_table = create_table_json(filename, args, len);
    add_database_new_table(curr_db->name_path, new_table);
//    cJSON_Delete(new_table);


    // Update database meta file
//    add_database_new_table(args, len);
//    current_db_t *curr_db = get_current_db();

//    // Create table info in json
//    cJSON *root = create_table_json(filename, args, len);
//    char *json_string = cJSON_Print(root);
//    fprintf(file, "%s\n", json_string);
//    cJSON_Delete(root);
//    free(json_string);


//    cJSON_Delete(new_table);
//    fclose(file);
}

cJSON *create_table_json(const char *table_name, char **args, size_t len) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "table_name", table_name);

    cJSON *columns = cJSON_CreateArray();
    for (size_t i=0; i<len; i+=2) {
        cJSON *column = cJSON_CreateObject();
        cJSON_AddStringToObject(column, "column_name", args[i]);
        cJSON_AddStringToObject(column, "type", args[i+1]);
        cJSON_AddItemToArray(columns, column);
    }
    cJSON_AddItemToObject(root, "columns", columns);

    return root;
}


void delete_table(const char *filename) {
    if (remove(filename) != 0) {
        fprintf(stderr, "Failed to delete table: %s \n", filename);
        assert(0);
    }
}

const char* create_filename(const char *filename, const char *ext) {
    memset(g_db_name, 0, DB_NAME_MAX * sizeof(char));
    strncpy(g_db_name, filename, strlen(filename));
    strncat(g_db_name, ext, strlen(ext));
    g_db_name[sizeof(g_db_name) - 1] = '\0';
    return g_db_name;
}

const char* create_filename_full_path(const char *base, const char *name, const char *ext) {
    memset(g_db_file_path, 0, PATH_MAX * sizeof(char));

    strncpy(g_db_file_path, base, strlen(base));
    strncat(g_db_file_path, "/", strlen("/")+1);

    const char *filename = create_filename(name, ext);
    strncat(g_db_file_path, filename, strlen(filename));
    g_db_file_path[sizeof(g_db_file_path) - 1] = '\0';
    return g_db_file_path;
}

const char* str_concat(const char *format, ...) {
    memset(g_db_file_path, 0, PATH_MAX * sizeof(char));

    va_list args;
    va_start(args, format);
    vsnprintf(g_db_file_path, PATH_MAX, format, args);
    va_end(args);

    return g_db_file_path;
}

