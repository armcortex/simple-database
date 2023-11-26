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

void create_database(const char *name) {
    FILE *file = fopen(name, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to create database: %s \n", name);
        assert(0);
    }

    // Get filename, and write in the beginning of the file
    splitter_t splitter = split_construct();
    size_t num_tokens;
    char** filenames = splitter.run(name, "/", &num_tokens);
    fprintf(file, "// Database: %s \n\n", filenames[num_tokens-1]);
    splitter.free(filenames, num_tokens);

    fclose(file);
}

void create_database_meta(const char *name) {
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

    FILE *file = fopen(name, "a");
    if (file == NULL) {
        fprintf(stderr, "Failed to write database: %s \n", name);
        cJSON_free(json_str);
        cJSON_Delete(json);
        assert(0);
    }
    fprintf(file, "%s", json_str);
    fclose(file);
    cJSON_free(json_str);
    cJSON_Delete(json);
}

void update_database_meta_table_cnt() {
}

void delete_database(const char *name) {
    if (remove(name) != 0) {
        fprintf(stderr, "Failed to delete database: %s \n", name);
        assert(0);
    }
}

void create_table(const char *name, char **args, size_t len) {
    FILE *file = fopen(name, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to create table: %s \n", name);
        assert(0);
    }

    // Show column on the first line
    for (size_t i=0; i<(len-2); i+=2) {
        fprintf(file, "%s, ", args[i]);
    }
    fprintf(file, "%s\n\n", args[len-2]);


    // Create json table

    // Update database meta file
//    add_database_new_table(args, len);
    current_db_t *curr_db = get_current_db();
    cJSON *root = create_table_json(curr_db->name, args, len);
    char *json_string = cJSON_Print(root);
    fprintf(file, "%s\n", json_string);
    cJSON_Delete(root);
    free(json_string);



    fclose(file);
}

cJSON *create_table_json(const char *name, char **args, size_t len) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "table_name", name);

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


void delete_table(const char *name) {
    if (remove(name) != 0) {
        fprintf(stderr, "Failed to delete table: %s \n", name);
        assert(0);
    }
}

const char* create_filename(const char *name, const char *ext) {
    memset(g_db_name, 0, DB_NAME_MAX * sizeof(char));
    strncpy(g_db_name, name, strlen(name));
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

