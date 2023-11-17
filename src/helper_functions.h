//
// Created by MCS51_M2 on 2023/11/16.
//

#ifndef SIMPLE_DATABASE_HELPER_FUNCTIONS_H
#define SIMPLE_DATABASE_HELPER_FUNCTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct {
    char** (*run)(const char* str, const char* delim, int* num_tokens);
    void (*free)(char** tokens, int count);
} Splitter_t;

Splitter_t split_construct();


#ifdef __cplusplus
}
#endif

#endif //SIMPLE_DATABASE_HELPER_FUNCTIONS_H
