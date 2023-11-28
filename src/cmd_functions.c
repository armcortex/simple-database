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
#include <dirent.h>
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

void insert_command_info() {
    fprintf(stdout, "insert <table_name> values <value1,value2,value3,...>\n");
}

void select_command_info() {
    fprintf(stdout, "select <column_names> from <table_name> (where <condition> ...) \n");
}

void create_database(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to create database: %s \n", filename);
        assert(0);
    }
    fclose(file);

    create_database_meta(filename);
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
    char *content = read_file(db_filename, 0);

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
    FILE *file = fopen(db_filename, "w");
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
    else {
        fprintf(stdout, "Delete database: %s \n", filename);
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
    fprintf(file, "%s\n", args[len-2]);
    fclose(file);

    // Update new table to database meta data
    current_db_t *curr_db = get_current_db();
    cJSON *new_table = create_table_json(filename, args, len);
    add_database_new_table(curr_db->name_path, new_table);
}

cJSON *create_table_json(const char *table_name, char **args, size_t len) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "table_name", table_name);
    cJSON_AddNumberToObject(root, "data_cnt", 0);

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

void insert_table_data(const char *filename_path, const char *table_name, char **datas, size_t len) {
    FILE *file = fopen(filename_path, "a");
    if (file == NULL) {
        fprintf(stderr, "Failed to insert data: %s \n", filename_path);
        assert(0);
    }

    for (size_t i=0; i<len-1; i++) {
        fprintf(file, "%s, ", datas[i]);
    }
    fprintf(file, "%s \n", datas[len-1]);
    fclose(file);

    current_db_t *curr_db = get_current_db();
    insert_table_update_database_meta(curr_db->name_path, table_name, 1);
}

void insert_table_update_database_meta(const char *db_filename, const char *table_name, size_t data_update_cnt) {
    char *content = read_file(db_filename, 0);

    // Parse the JSON content
    cJSON *root = cJSON_Parse(content);
    free(content);
    if (root == NULL) {
        fprintf(stderr, "Failed to parse JSON\n");
        assert(0);
    }

    // find "tables" array
    cJSON *tables = cJSON_GetObjectItemCaseSensitive(root, "tables");
    if (!cJSON_IsArray(tables)) {
        fprintf(stderr, "\"tables\" is not an array\n");
        cJSON_Delete(root);
        return;
    }

    // traverse all array
    cJSON *table;
    cJSON_ArrayForEach(table, tables) {
        cJSON *name = cJSON_GetObjectItemCaseSensitive(table, "table_name");
        if (cJSON_IsString(name) && (strcmp(name->valuestring, table_name) == 0)) {
            // update "data_cnt"
            cJSON *dataCntItem = cJSON_GetObjectItem(table, "data_cnt");
            if (dataCntItem != NULL) {
                cJSON_SetIntValue(dataCntItem, dataCntItem->valueint + data_update_cnt);
            }
            break;
        }
    }

    // Write json back to the file
    FILE *file = fopen(db_filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to open database file:: %s\n", db_filename);
        cJSON_Delete(root);
        assert(0);
    }
    char *modified_content = cJSON_Print(root);
    fprintf(file, "%s", modified_content);
    fclose(file);
    cJSON_free(modified_content);
    cJSON_Delete(root);
}

void delete_table(const char *filename) {
    if (remove(filename) != 0) {
        fprintf(stderr, "Failed to delete table: %s \n", filename);
        assert(0);
    }
    else {
        fprintf(stdout, "Delete table: %s \n", filename);
    }
}

void delete_table_all(const char *db_base_path) {
    DIR *d = opendir(db_base_path);
    struct dirent *dir;
    char del_path[PATH_MAX] = {0};

    if (d) {
        dir = readdir(d);
        while (dir != NULL) {
            if (strstr(dir->d_name, ".csv") != NULL) {
                snprintf(del_path, sizeof(del_path), "%s/%s", db_base_path, dir->d_name);
                delete_table(del_path);
            }
            dir = readdir(d);
        }
        closedir(d);
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

