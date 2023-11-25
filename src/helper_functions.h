//
// Created by MCS51_M2 on 2023/11/16.
//

#ifndef SIMPLE_DATABASE_HELPER_FUNCTIONS_H
#define SIMPLE_DATABASE_HELPER_FUNCTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

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

#ifdef __cplusplus
}
#endif

#endif //SIMPLE_DATABASE_HELPER_FUNCTIONS_H
