//
// Created by MCS51_M2 on 2023/11/22.
//

#include <stdio.h>
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

static char db_file_path[PATH_MAX] = {0};
static char db_name[DB_NAME_MAX] = {0};

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

    // Write to file
    char *json_str = cJSON_Print(json);
    if (json_str == NULL) {
        fprintf(stderr, "Failed to init cJSON_Print() \n");
        assert(0);
    }

    FILE *file = fopen(name, "a");
    if (file == NULL) {
        fprintf(stderr, "Failed to write database: %s \n", name);
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

void create_table(const char *name) {
    FILE *file = fopen(name, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to create table: %s \n", name);
        assert(0);
    }

    fprintf(file, "This is a table\n");

    fclose(file);


    update_database_meta_table_cnt();
}

void delete_table(const char *name) {
    if (remove(name) != 0) {
        fprintf(stderr, "Failed to delete table: %s \n", name);
        assert(0);
    }
}

const char* create_filename(const char *name, const char *ext) {
    memset(db_name, 0, DB_NAME_MAX * sizeof(char));
    strncpy(db_name, name, strlen(name));
    strncat(db_name, ext, strlen(ext));
    db_name[sizeof(db_name) - 1] = '\0';
    return db_name;
}

const char* create_filename_full_path(const char *base, const char *name, const char *ext) {
    memset(db_file_path, 0, PATH_MAX * sizeof(char));

    strncpy(db_file_path, base, strlen(base));
    strncat(db_file_path, "/", strlen("/")+1);

    const char *filename = create_filename(name, ext);
    strncat(db_file_path, filename, strlen(filename));
    db_file_path[sizeof(db_file_path) - 1] = '\0';
    return db_file_path;
}

const char* str_concat(const char *format, ...) {
    memset(db_file_path, 0, PATH_MAX * sizeof(char));

    va_list args;
    va_start(args, format);
    vsnprintf(db_file_path, PATH_MAX, format, args);
    va_end(args);

    return db_file_path;
}

