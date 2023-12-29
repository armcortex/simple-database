//
// Created by MCS51_M2 on 2023/11/22.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>
#include <stdarg.h>
#include <time.h>
#include <dirent.h>
#include <cJSON.h>
#include <regex.h>

#include "cmd_functions.h"
#include "db_config.h"
#include "helper_functions.h"
#include "database.h"

static char g_db_file_path[PATH_MAX] = {0};
static char g_db_name[DB_NAME_MAX] = {0};


table_data_t* table_data_init(size_t len) {
    table_data_t *t = (table_data_t*)malloc(sizeof(table_data_t));
    if (t == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        assert(0);
    }

    t->types = (table_data_enum_t*)malloc(len * sizeof(table_data_enum_t));
    if (t->types == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        assert(0);
    }

    t->table_column_names = (char**)malloc(len * sizeof(char*));
    if (t->table_column_names == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        assert(0);
    }
    for (size_t i=0; i<len; i++) {
        t->table_column_names[i] = (char*)calloc(CELL_TEXT_MAX,  sizeof(char));
        if (t->table_column_names[i] == NULL) {
            fprintf(stderr, "Failed to allocate memory.\n");
            assert(0);
        }
    }

    t->len = len;
    t->rows = NULL;
    return t;
}

void table_data_close(table_data_t *t) {
    free(t->types);
    free(t->table_column_names);
    t->types = NULL;
    t->table_column_names = NULL;
    if (t->rows != NULL) {
        free(t->rows);
        t->rows = NULL;
    }
    free(t);
    t = NULL;
}

void table_data_add_type(table_data_t *t, const char *type, size_t idx) {
    if (idx >= t->len) {
        fprintf(stderr, "Table type out of range, should less than %zu, idx: %zu", t->len, idx);
        assert(0);
    }

    if (strncmp(type, "STRING", 6) == 0) {
        t->types[idx] = TABLE_STRING;
    }
    else if (strncmp(type, "INT", 3) == 0) {
        t->types[idx] = TABLE_INT;
    }
    else if (strncmp(type, "FLOAT", 5) == 0) {
        t->types[idx] = TABLE_FLOAT;
    }
    else {
        fprintf(stderr, "Table type not supported: %s\n", type);
        assert(0);
    }
}

void table_data_add_column_name(table_data_t *t, const char *column_name, size_t idx) {
    if (idx >= t->len) {
        fprintf(stderr, "Table type out of range, should less than %zu, idx: %zu", t->len, idx);
        assert(0);
    }
    strncpy(t->table_column_names[idx], column_name, strlen(column_name));
}

table_row_t* table_data_create_row_node(char **data, size_t len) {
    table_row_t *t = (table_row_t*)malloc(sizeof(table_row_t*));
    if (t == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        assert(0);
    }
    t->next = NULL;

    t->data = (char**)malloc(len * sizeof(char*));
    for (size_t i=0; i<len; i++) {
        t->data[i] = (char*)calloc(CELL_TEXT_MAX,  sizeof(char));
        if (t->data[i] == NULL) {
            fprintf(stderr, "Failed to allocate memory.\n");
            assert(0);
        }

        strncpy(t->data[i], data[i], strlen(data[i]));
    }
    return t;
}

void table_data_insert_row_data(table_data_t *t, char **data, size_t data_len) {
    if (t->rows == NULL) {
        t->rows = table_data_create_row_node(data, data_len);
        return;
    }

    table_row_t *last = t->rows;
    while (last->next) {
        last = last->next;
    }
    last->next = table_data_create_row_node(data, data_len);
}

void basic_command_info() {
    fprintf(stdout, "All Support commands: \n");
    fprintf(stdout, "\t help: \n");
    fprintf(stdout, "\t exit: \n");
    fprintf(stdout, "\t create: \n");
    fprintf(stdout, "\t use: \n");
    fprintf(stdout, "\t drop: \n");
    fprintf(stdout, "\t select: \n");
}

void create_command_info() {
    fprintf(stdout, "Create sub-commands: \n");
    fprintf(stdout, "\t database <database name> \n");
    fprintf(stdout, "\t table <table name> (<column name> <column type> ...) \n");
}

void delete_command_info() {
    fprintf(stdout, "Delete sub-commands: \n");
    fprintf(stdout, "\t database <database name> \n");
    fprintf(stdout, "\t table <table name> \n");
}

void use_command_info() {
    fprintf(stdout, "Use sub-commands: \n");
    fprintf(stdout, "\t <database name> \n");
}

void insert_command_info() {
    fprintf(stdout, "insert <table_name> values <value1,value2,value3,...>\n");
}

void select_command_info() {
    fprintf(stdout, "select <column_names> from <table_name> (where <condition> ...) \n");
}

void create_database(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to create database: %s \n", filename);
        assert(0);
    }
    fclose(file);

    create_database_meta(filename);
}

void create_database_meta(const char *filename) {
    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        fprintf(stderr, "Failed to create JSON object \n");
        assert(0);
    }

    // Calc time
    char formatted_time[50] = {0};
    time_t current_time = time(NULL);
    strftime(formatted_time, sizeof(formatted_time), "%Y-%m-%d %H:%M:%S",
            localtime(&current_time));

    // Create json element
    cJSON_AddStringToObject(json, "time", formatted_time);
    cJSON_AddNumberToObject(json, "timestamp", (int)current_time);
    cJSON_AddNumberToObject(json, "table_cnt", 0);

    // Create an empty array
    cJSON *tables_array = cJSON_CreateArray();
    if (tables_array == NULL) {
        fprintf(stderr, "Failed to create tables array \n");
        cJSON_Delete(json);
        assert(0);
    }
    cJSON_AddItemToObject(json, "tables", tables_array);

    // Write to file
    char *json_str = cJSON_Print(json);
    if (json_str == NULL) {
        fprintf(stderr, "Failed to init cJSON_Print() \n");
        cJSON_Delete(json);
        assert(0);
    }

    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        fprintf(stderr, "Failed to write database: %s \n", filename);
        cJSON_free(json_str);
        cJSON_Delete(json);
        assert(0);
    }
    fprintf(file, "%s", json_str);
    fclose(file);
    cJSON_free(json_str);
    cJSON_Delete(json);
}

void add_database_new_table(const char *db_filename, cJSON *new_table) {
    u_int32_t res_lines = 0;
    char *content = read_file(db_filename, 0, &res_lines);

    // Parse the JSON content
    cJSON *json = cJSON_Parse(content);
    free(content);
    content = NULL;
    if (json == NULL) {
        fprintf(stderr, "Failed to parse JSON\n");
        assert(0);
    }

    // Get the tables array
    cJSON *tables = cJSON_GetObjectItem(json, "tables");
    if (!cJSON_IsArray(tables)) {
        fprintf(stderr, "\"table\" is not an array\n");
        cJSON_Delete(json);
        assert(0);
    }

    // Add the new table to the tables array
    cJSON_AddItemToArray(tables, new_table);

    // Update table_cnt
    cJSON *table_cnt = cJSON_GetObjectItem(json, "table_cnt");
    if (cJSON_IsNumber(table_cnt)) {
        table_cnt->valuedouble = cJSON_GetArraySize(tables); // Update count
    }

    // Write json back to the file
    FILE *file = fopen(db_filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to open database file:: %s\n", db_filename);
        cJSON_Delete(json);
        assert(0);
    }
    char *modified_content = cJSON_Print(json);
    fprintf(file, "%s", modified_content);
    fclose(file);
    cJSON_free(modified_content);
    cJSON_Delete(json);
}

void delete_database(const char *filename) {
    if (remove(filename) != 0) {
        fprintf(stderr, "Failed to delete database: %s \n", filename);
        assert(0);
    }
    else {
        fprintf(stdout, "Delete database: %s \n", filename);
    }
}

void create_table(const char *filename_path, const char *filename, char **args, size_t len) {
    FILE *file = fopen(filename_path, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to create table: %s \n", filename_path);
        assert(0);
    }

    // Show column on the first line
    for (size_t i=0; i<(len-2); i+=2) {
        fprintf(file, "%s,", args[i]);
    }
    fprintf(file, "%s\n", args[len-2]);
    fclose(file);

    // Update new table to database meta data
    current_db_t *curr_db = get_current_db();
    cJSON *new_table = create_table_json(filename, args, len);
    add_database_new_table(curr_db->name_path, new_table);
}

cJSON *create_table_json(const char *table_name, char **args, size_t len) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "table_name", table_name);
    cJSON_AddNumberToObject(root, "data_cnt", 0);

    cJSON *columns = cJSON_CreateArray();
    for (size_t i=0; i<len; i+=2) {
        cJSON *column = cJSON_CreateObject();
        cJSON_AddStringToObject(column, "column_name", args[i]);
        cJSON_AddStringToObject(column, "type", args[i+1]);
        cJSON_AddItemToArray(columns, column);
    }
    cJSON_AddItemToObject(root, "columns", columns);

    return root;
}

void insert_table_data(const char *filename_path, const char *table_name, char **datas, size_t len) {
    FILE *file = fopen(filename_path, "a");
    if (file == NULL) {
        fprintf(stderr, "Failed to insert data: %s \n", filename_path);
        assert(0);
    }

    for (size_t i=0; i<len-1; i++) {
        fprintf(file, "%s,", datas[i]);
    }
    fprintf(file, "%s\n", datas[len-1]);
    fclose(file);

    current_db_t *curr_db = get_current_db();
    insert_table_update_database_meta(curr_db->name_path, table_name, 1);
}

void insert_table_update_database_meta(const char *db_filename, const char *table_name, size_t data_update_cnt) {
    u_int32_t res_lines = 0;
    char *content = read_file(db_filename, 0, &res_lines);

    // Parse the JSON content
    cJSON *root = cJSON_Parse(content);
    free(content);
    content = NULL;
    if (root == NULL) {
        fprintf(stderr, "Failed to parse JSON\n");
        assert(0);
    }

    // find "tables" array
    cJSON *tables = cJSON_GetObjectItemCaseSensitive(root, "tables");
    if (!cJSON_IsArray(tables)) {
        fprintf(stderr, "\"tables\" is not an array\n");
        cJSON_Delete(root);
        assert(0);
    }

    // traverse all array
    cJSON *table;
    cJSON_ArrayForEach(table, tables) {
        cJSON *name = cJSON_GetObjectItemCaseSensitive(table, "table_name");
        if (cJSON_IsString(name) && (strcmp(name->valuestring, table_name) == 0)) {
            // update "data_cnt"
            cJSON *dataCntItem = cJSON_GetObjectItem(table, "data_cnt");
            if (dataCntItem != NULL) {
                cJSON_SetIntValue(dataCntItem, dataCntItem->valueint + data_update_cnt);
            }
            break;
        }
    }

    // Write json back to the file
    FILE *file = fopen(db_filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to open database file:: %s\n", db_filename);
        cJSON_Delete(root);
        assert(0);
    }
    char *modified_content = cJSON_Print(root);
    fprintf(file, "%s", modified_content);
    fclose(file);
    cJSON_free(modified_content);
    cJSON_Delete(root);
}

void delete_table(const char *filename) {
    if (remove(filename) != 0) {
        fprintf(stderr, "Failed to delete table: %s \n", filename);
        assert(0);
    }
    else {
        fprintf(stdout, "Delete table: %s \n", filename);
    }
}

void delete_table_all(const char *db_base_path) {
    DIR *d = opendir(db_base_path);
    struct dirent *dir;
    char del_path[PATH_MAX] = {0};

    if (d) {
        dir = readdir(d);
        while (dir != NULL) {
            if (strstr(dir->d_name, ".csv") != NULL) {
                snprintf(del_path, sizeof(del_path), "%s/%s", db_base_path, dir->d_name);
                delete_table(del_path);
            }
            dir = readdir(d);
        }
        closedir(d);
    }
}

uint8_t* find_parsed_colummn_name_idx(table_data_t *table_data, select_parsed_data_t *parsed_data, size_t parsed_len, uint8_t *res_len) {
    size_t col_len = table_data->len;
    uint8_t res_idx = 0;
    uint8_t *res = (uint8_t*)malloc(parsed_len * sizeof(uint8_t));
    memset(res, 0, parsed_len * sizeof(uint8_t));

    // Split column_names
    splitter_t splitter = split_construct();
    size_t col_name_num = 0;
    char **col_names = splitter.run(parsed_data[0].args, ",", &col_name_num);

    // Find selected column_name index
    for (size_t i=0; i<col_len; i++) {
        for (size_t j=0; j<col_name_num; j++) {
            size_t col_name_len = strlen(table_data->table_column_names[i]);
            if (strncmp(table_data->table_column_names[i], col_names[j], col_name_len) == 0) {
                res[res_idx] = j;
                res_idx++;
            }
        }
    }
    *res_len = res_idx;

    splitter.free(col_names, col_name_num);
    return res;
}

void select_load_table_data(table_data_t *table_data, char *table_name_path, select_parsed_data_t *parsed_data, size_t parsed_len) {
    // Read table content
    u_int32_t res_lines = 0;
    char *table = read_file(table_name_path, 1, &res_lines);

    // Split into lines
    splitter_t lines_splitter = split_construct();
    size_t lines_num_tokens;
    char** lines = lines_splitter.run(table, "\n", &lines_num_tokens);

    // Split into cell, save to table_struct
    uint8_t selected_col_name_len = 0;
    for (size_t i=0; i<lines_num_tokens; i++) {
        splitter_t cells_splitter = split_construct();
        size_t cells_num_tokens;
        char** cells = cells_splitter.run(lines[i], ",", &cells_num_tokens);

        // filter
        // TODO: Just use parsed_data (containe all user wanted colummn name), and filter out the data
        // find wanted cells index, based on table meta info of column and compare to `parsed_data` to get wanted index
        uint8_t *selected_col_name_idxs = find_parsed_colummn_name_idx(table_data, parsed_data, parsed_len, &selected_col_name_len);

        // Get filtered data
        char **tmp_cells = (char**)malloc(selected_col_name_len * sizeof(char*));
        memset(tmp_cells, 0, selected_col_name_len * sizeof(char*));
        for (uint8_t j=0; j<selected_col_name_len; j++) {
            uint8_t idx = selected_col_name_idxs[j];
            uint8_t str_len = strlen(cells[idx]) + 1;
            tmp_cells[j] = (char*)malloc(str_len * sizeof(char));
            strncpy(tmp_cells[j], cells[idx], str_len);
        }

        // insert data
        table_data_insert_row_data(table_data, tmp_cells, selected_col_name_len);


        // Free
        for (size_t j=0; j<selected_col_name_len; j++) {
            free(tmp_cells[j]);
        }
        free(tmp_cells);
        cells_splitter.free(cells, cells_num_tokens);
    }
    table_data->len = selected_col_name_len;

    // Close
    lines_splitter.free(lines, lines_num_tokens);
    free(table);
    table = NULL;
}

table_data_t* select_load_table_column_names(const char *table_name) {
    current_db_t *db = get_current_db();

    uint32_t res_lines = 0;
    char *content = read_file(db->name_path, 0, &res_lines);

    // Parse the JSON content
    cJSON *root = cJSON_Parse(content);
    free(content);
    content = NULL;
    if (root == NULL) {
        fprintf(stderr, "Failed to parse JSON\n");
        assert(0);
    }

    // find "tables" array
    cJSON *tables = cJSON_GetObjectItemCaseSensitive(root, "tables");
    if (!cJSON_IsArray(tables)) {
        fprintf(stderr, "\"tables\" is not an array\n");
        cJSON_Delete(root);
        assert(0);
    }

    table_data_t *table_data = NULL;
    int column_len;
    cJSON *table;
    size_t type_idx = 0;
    cJSON_ArrayForEach(table, tables) {
        cJSON *name = cJSON_GetObjectItemCaseSensitive(table, "table_name");
        if (cJSON_IsString(name) && strcmp(name->valuestring, table_name) == 0) {
            cJSON *columns = cJSON_GetObjectItemCaseSensitive(table, "columns");
            if (!cJSON_IsArray(columns)) {
                fprintf(stderr, "\"columns\" is not an array for table %s\n", table_name);
                break;
            }

            // Create table info
            column_len = cJSON_GetArraySize(columns);
            table_data = table_data_init(column_len);

            cJSON *column;
            cJSON_ArrayForEach(column, columns) {
                cJSON *column_name = cJSON_GetObjectItemCaseSensitive(column, "column_name");
                cJSON *type = cJSON_GetObjectItemCaseSensitive(column, "type");

                if (cJSON_IsString(column_name) && cJSON_IsString(type)) {
                    table_data_add_type(table_data, type->valuestring, type_idx);
                    table_data_add_column_name(table_data, column_name->valuestring, type_idx);
//                    logger_str("Column name: %s, Type: %s\n", column_name->valuestring, type->valuestring);
                    type_idx += 1;
                }
            }
            break;  // Once the specified table is found, no need to check other tables.
        }
    }

    cJSON_Delete(root);
    return table_data;
}

void select_table_display(table_data_t *table_data) {
    size_t len = table_data->len;

    // Print column names
    for (size_t i=0; i<len-1; i++) {
        fprintf(stdout, "%s,", table_data->table_column_names[i]);
    }
    fprintf(stdout, "%s\n", table_data->table_column_names[len-1]);

    // Print data
    table_row_t *head = table_data->rows;
    while (head) {
        for (size_t i=0; i<len-1; i++) {
            fprintf(stdout, "%s,", head->data[i]);
        }
        fprintf(stdout, "%s", head->data[len-1]);
        fprintf(stdout, "\n");

        head = head->next;
    }
}

void select_table_close(table_data_t *table_data) {
    table_data_close(table_data);
}

const char* create_filename(const char *filename, const char *ext) {
    memset(g_db_name, 0, DB_NAME_MAX * sizeof(char));
    strncpy(g_db_name, filename, strlen(filename));
    strncat(g_db_name, ext, strlen(ext));
    g_db_name[sizeof(g_db_name) - 1] = '\0';
    return g_db_name;
}

const char* create_filename_full_path(const char *base, const char *name, const char *ext) {
    memset(g_db_file_path, 0, PATH_MAX * sizeof(char));

    strncpy(g_db_file_path, base, strlen(base));
    strncat(g_db_file_path, "/", strlen("/")+1);

    const char *filename = create_filename(name, ext);
    strncat(g_db_file_path, filename, strlen(filename));
    g_db_file_path[sizeof(g_db_file_path) - 1] = '\0';
    return g_db_file_path;
}

const char* str_concat(const char *format, ...) {
    memset(g_db_file_path, 0, PATH_MAX * sizeof(char));

    va_list args;
    va_start(args, format);
    vsnprintf(g_db_file_path, PATH_MAX, format, args);
    va_end(args);

    return g_db_file_path;
}

bool check_table_exist(const char *table_name, char *table_name_path) {
    current_db_t *db = get_current_db();
    snprintf(table_name_path, PATH_MAX, "%s/%s.csv", db->folder_path, table_name);
    return exist_file(table_name_path);
}
