//
// Created by MCS51_M2 on 2023/11/16.
//

#ifndef SIMPLE_DATABASE_HELPER_FUNCTIONS_H
#define SIMPLE_DATABASE_HELPER_FUNCTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>


#define DB_ASSERT(x) \
    do { \
        if (!(x)) { \
            fflush(stdout); \
            fprintf(stderr, "DB_ASSERT: %s:%d: %s\n", __FILE__, __LINE__, #x); \
            abort(); \
        } \
    } while (0)


typedef struct {
    char** (*run)(const char* str, const char* delim, size_t *num_tokens);
    void (*free)(char** tokens, size_t count);
} splitter_t;

splitter_t split_construct(void);

void check_current_path(void);
void create_folder(const char *name);
void remove_folder(const char *name);
bool exist_folder(const char *name);
bool exist_file(const char *name);
char* read_file(const char* filename, uint32_t skip_lines, uint32_t *res_lines);

void delete_semicolon(char *s);

void logger_str(bool with_time, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif //SIMPLE_DATABASE_HELPER_FUNCTIONS_H
