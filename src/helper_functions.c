//
// Created by MCS51_M2 on 2023/11/16.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

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
