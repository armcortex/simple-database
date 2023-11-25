//
// Created by MCS51_M2 on 2023/11/25.
//

#include <string.h>

#include "database.h"


static current_db_t g_current_db;

current_db_t* get_current_db() {
    return &g_current_db;
}

void update_current_db(const char *name) {
    memset(&g_current_db, 0, sizeof(current_db_t));
    g_current_db.len = strlen(name);
    strncpy(g_current_db.name, name, g_current_db.len);
}
