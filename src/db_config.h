//
// Created by MCS51_M2 on 2023/11/22.
//

#ifndef SIMPLE_DATABASE_DB_CONFIG_H
#define SIMPLE_DATABASE_DB_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif


#define PAGE_SIZE           4096
#define TABLE_MAX_PAGES     100

#define WORKSPACE_PATH_BASE             "../"
#define DB_WORKING_FOLDER               "DB_DATA"
#define WORKSPACE_PATH_FULL             WORKSPACE_PATH_BASE DB_WORKING_FOLDER

#define DB_NAME_MAX                     (256)
#define CELL_TEXT_MAX                   (32)

#ifdef __cplusplus
}
#endif

#endif //SIMPLE_DATABASE_DB_CONFIG_H
