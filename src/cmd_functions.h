//
// Created by MCS51_M2 on 2023/11/22.
//

#ifndef SIMPLE_DATABASE_CMD_FUNCTIONS_H
#define SIMPLE_DATABASE_CMD_FUNCTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define DB_NAME_MAX         (256)

void basic_command_info(void);
void create_command_info(void);
void create_database(const char *name);
const char* create_filename(char *name, char *ext);
const char* create_filename_full_path(char *base, char *name, char *ext);

#ifdef __cplusplus
}
#endif

#endif //SIMPLE_DATABASE_CMD_FUNCTIONS_H
