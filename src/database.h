//
// Created by MCS51_M2 on 2023/11/25.
//

#ifndef SIMPLE_DATABASE_DATABASE_H
#define SIMPLE_DATABASE_DATABASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "db_config.h"

typedef struct current_db_t {
    char name[DB_NAME_MAX];
    size_t len;
} current_db_t;

current_db_t* get_current_db();
void update_current_db(const char *name);

#ifdef __cplusplus
}
#endif


#endif //SIMPLE_DATABASE_DATABASE_H
