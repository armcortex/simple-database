//
// Created by MCS51_M2 on 2023/11/25.
//

#include <string.h>
#include <stdbool.h>

#include "database.h"


static current_db_t g_current_db;

const current_db_t * get_current_db() {
    return &g_current_db;
}

void clean_current_db() {
    memset(&g_current_db, 0, sizeof(current_db_t));
}

bool check_current_db_exist() {
    return (g_current_db.len != 0);
}

void update_current_db(const char *name, const char *filename_path, const char *folder_path) {
    memset(&g_current_db, 0, sizeof(current_db_t));
    g_current_db.len = strlen(name);
    strncpy(g_current_db.name, name, g_current_db.len);
    strncpy(g_current_db.name_path, filename_path, strlen(filename_path));
    strncpy(g_current_db.folder_path, folder_path, strlen(folder_path));
}
