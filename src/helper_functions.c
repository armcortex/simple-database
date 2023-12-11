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
#include <assert.h>
#include <stdarg.h>
#include <time.h>

#include "helper_functions.h"


static void split_free(char** tokens, size_t count) {
    for (size_t i=0; i<count; i++) {
        free(tokens[i]);
        tokens[i] = NULL;
    }
    free(tokens);
    tokens = NULL;
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
        str_copy = NULL;
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
                str_copy = NULL;
                exit(EXIT_FAILURE);
            }
            tokens = new_tokens;
        }
        tokens[count] = strdup(token);
        if (!tokens[count]) {
            fprintf(stderr, "Failed to allocate memory.\n");
            split_free(tokens, count);
            free(str_copy);
            str_copy = NULL;
            exit(EXIT_FAILURE);
        }
        count++;
        token = strtok(NULL, delim);
    }

    *num_tokens = count;
    free(str_copy);
    str_copy = NULL;
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

void create_folder(const char *name) {
    struct stat st = {0};

    if (stat(name, &st) == -1) {
        if (mkdir(name, 0700) == -1) { // 0700 permissions - owner can read, write, and execute
            fprintf(stderr, "Failed to create folder: %s \n", name);
        }
    }
    else {
        fprintf(stdout, "Folder `%s` already exists\n", name);
    }
}

void remove_folder(const char *name) {
    struct stat st = {0};

    if (exist_folder(name)) {
        if (rmdir(name) == -1) {
            fprintf(stderr, "Failed to remove folder: %s \n", name);
        }
    } else {
        fprintf(stdout, "Folder %s not exist\n", name);
    }
}

bool exist_folder(const char *name) {
    struct stat st = {0};
    return (stat(name, &st) == 0 && S_ISDIR(st.st_mode));
}

bool exist_file(const char *name) {
    struct stat st = {0};
    return (stat(name, &st) == 0);
}

char* read_file(const char* filename, uint32_t skip_lines, uint32_t *res_lines) {
    const int MAX_LINE_LENGTH = 1024;

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        assert(0);
    }

    // Skip lines
    char line[MAX_LINE_LENGTH];
    while (skip_lines > 0 && fgets(line, MAX_LINE_LENGTH, file) != NULL) {
        skip_lines--;
    }

    // Read content
    char *content = NULL;
    size_t content_length = 0;
    while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
        size_t line_length = strlen(line);
        char *new_content = realloc(content, content_length + line_length + 1);
        if (new_content == NULL) {
            fprintf(stderr, "Failed to allocate memory\n");
            free(content);
            content = NULL;
            fclose(file);
            assert(0);
        }
        content = new_content;
        strncpy(content + content_length, line, line_length + 1);
        content_length += line_length;
        (*res_lines) += 1;
    }

    if (content != NULL) {
        content[content_length] = '\0';
    }
    fclose(file);

    return content;
}

void logger_str(bool with_time, const char *format, ...) {
    const char *filename = "../logger_str.txt";
    FILE *f_log = fopen(filename, "a");
    if (f_log == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        assert(0);
    }

    if (with_time) {
        // Calc time
        char formatted_time[30] = {0};
        time_t current_time = time(NULL);
        strftime(formatted_time, sizeof(formatted_time), "[%Y-%m-%d %H:%M:%S] - ",
                 localtime(&current_time));

        // Write to file
        fprintf(f_log, "%s", formatted_time);
    }

    // Args
    va_list args;
    va_start(args, format);
    vfprintf(f_log, format, args);
    va_end(args);

    fclose(f_log);
}