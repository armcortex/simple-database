//
// Created by MCS51_M2 on 2023/11/22.
//

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>

#include "cmd_functions.h"
#include "db_config.h"

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
    fprintf(stdout, "Create commands: \n");
    fprintf(stdout, "\t database: \n");
    fprintf(stdout, "\t table: \n");
}

void create_database(const char *name) {
    FILE *file = fopen(name, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to create database: %s \n", name);
        assert(0);
    }
    fprintf(file, "This is a test.\n");
    fclose(file);
}

void delete_database(const char *name) {
    if (remove(name) != 0) {
        fprintf(stderr, "Failed to delete filename: %s \n", name);
        assert(0);
    }
}

const char* create_filename(char *name, char *ext) {
    memset(db_name, 0, DB_NAME_MAX * sizeof(char));
    strncpy(db_name, name, strlen(name));
    strncat(db_name, ext, strlen(ext));
    db_name[sizeof(db_name) - 1] = '\0';
    return db_name;
}

const char* create_filename_full_path(char *base, char *name, char *ext) {
    memset(db_file_path, 0, PATH_MAX * sizeof(char));

    strncpy(db_file_path, base, strlen(base));
    strncat(db_file_path, "/", strlen("/"));

    const char *filename = create_filename(name, ext);
    strncat(db_file_path, filename, strlen(filename));
    db_file_path[sizeof(db_file_path) - 1] = '\0';
    return db_file_path;
}
