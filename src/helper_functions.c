//
// Created by MCS51_M2 on 2023/11/16.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "helper_functions.h"


static void split_free(char** tokens, size_t count) {
    for (size_t i=0; i<count; i++) {
        free(tokens[i]);
    }
    free(tokens);
}

static char** split_run(const char* str, const char* delim, size_t *num_tokens) {
    char* str_copy = strdup(str);
    if (!str_copy) {
        fprintf(stderr, "Failed to allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    size_t capacity = 10;
    char** tokens = malloc(capacity * sizeof(char*));
    if (!tokens) {
        fprintf(stderr, "Failed to allocate memory.\n");
        free(str_copy);
        exit(EXIT_FAILURE);
    }

    size_t count = 0;
    char* token = strtok(str_copy, delim);
    while (token != NULL) {
        if (count >= capacity) {
            capacity *= 2;
            char** new_tokens = realloc(tokens, capacity * sizeof(char*));
            if (!new_tokens) {
                fprintf(stderr, "Failed to allocate memory.\n");
                split_free(tokens, count);
                free(str_copy);
                exit(EXIT_FAILURE);
            }
            tokens = new_tokens;
        }
        tokens[count] = strdup(token);
        if (!tokens[count]) {
            fprintf(stderr, "Failed to allocate memory.\n");
            split_free(tokens, count);
            free(str_copy);
            exit(EXIT_FAILURE);
        }
        count++;
        token = strtok(NULL, delim);
    }

    *num_tokens = count;
    free(str_copy);
    return tokens;
}

splitter_t split_construct() {
    splitter_t s;
    s.run = split_run;
    s.free = split_free;
    return s;
}

void check_current_path() {
    char *cwd;
    char buffer[PATH_MAX];  // PATH_MAX is a constant defined in <limits.h>

    cwd = getcwd(buffer, sizeof(buffer));
    if (cwd == NULL) {
        fprintf(stderr, "getcwd() error");
    }

    fprintf(stdout, "Current working directory: %s\n", cwd);
}

void create_folder(char *folder_name) {
    struct stat st = {0};

    if (stat(folder_name, &st) == -1) {
        if (mkdir(folder_name, 0700) == -1) { // 0700 permissions - owner can read, write, and execute
            fprintf(stderr, "Failed to create folder: %s \n", folder_name);
        }
    }
    else {
        printf("Folder `%s` already exists\n", folder_name);
    }
}
