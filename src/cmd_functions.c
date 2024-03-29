//
// Created by MCS51_M2 on 2023/11/22.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdarg.h>
#include <time.h>
#include <dirent.h>
#include <cJSON.h>
#include <regex.h>
#include <stdbool.h>

#include "cmd_functions.h"
#include "helper_functions.h"
#include "parser.h"
#include "database.h"
#include "rpn.h"
#include "inputs.h"


table_data_t *table_data_init(size_t col_len, size_t row_len) {
    table_data_t *t = (table_data_t*)malloc(sizeof(table_data_t));
    if (t == NULL) {
        DB_ASSERT(!"Failed to allocate memory.\n");
    }

    // Columns
    t->col_enable_cnt = 0;
    t->col_len = col_len;
    t->cols = (table_col_t*)calloc(col_len, sizeof(table_col_t));
    if (t->cols == NULL) {
        DB_ASSERT(!"Failed to allocate memory.\n");
    }
    for (size_t i=0; i < col_len; i++) {
        t->cols[i].enable = false;
    }

    // Rows
    t->row_len = row_len;
    t->rows = NULL;
    return t;
}

void table_data_close(table_data_t *t) {
    if (t->cols != NULL) {
        free(t->cols);
        t->cols = NULL;
    }

    if (t->rows != NULL) {
        table_row_t* curr = t->rows;
        table_row_t* next;
        while(curr != NULL) {
            next = curr->next;
            free(curr->data);
            free(curr);
            curr = next;
        }
    }

    free(t);
    t = NULL;
}

void table_data_add_type(table_data_t *t, const char *type, size_t idx) {
    if (idx >= t->col_len) {
        fprintf(stderr, "Table type out of range, should less than %zu, idx: %zu", t->col_len, idx);
        DB_ASSERT(0);
    }

    if (strncmp(type, "STRING", 6) == 0) {
        t->cols[idx].type = TABLE_STRING;
    }
    else if (strncmp(type, "INT", 3) == 0) {
        t->cols[idx].type = TABLE_INT;
    }
    else if (strncmp(type, "FLOAT", 5) == 0) {
        t->cols[idx].type = TABLE_FLOAT;
    }
    else {
        fprintf(stderr, "Table type not supported: %s\n", type);
        DB_ASSERT(0);
    }
}

void table_data_add_column_name(table_data_t *t, const char *column_name, size_t idx) {
    if (idx >= t->col_len) {
        fprintf(stderr, "Table type out of range, should less than %zu, idx: %zu", t->col_len, idx);
        DB_ASSERT(0);
    }
    strncpy(t->cols[idx].name, column_name, CELL_TEXT_MAX-1);
    t->cols[idx].name[CELL_TEXT_MAX-1] = '\0';
}

table_row_t* table_data_create_row_node(char **data, size_t len) {
    table_row_t *row = (table_row_t*)calloc(1, sizeof(table_row_t));
    if (row == NULL) {
        DB_ASSERT(!"Failed to allocate memory.\n");
    }

    row->enable = false;
    row->next = NULL;
    row->data = (row_cell_t*)calloc(len, sizeof(row_cell_t));
    if (row->data == NULL) {
        DB_ASSERT(!"Failed to allocate memory.\n");
    }
    for (size_t i=0; i<len; i++) {
        strncpy(row->data[i].cell, data[i], CELL_TEXT_MAX-1);
        row->data[i].cell[CELL_TEXT_MAX-1] = '\0';
    }

    return row;
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

bool basic_fn(char **args, size_t args_len) {
    (void)args;
    (void)args_len;

    basic_command_info();
    return true;
}

bool basic_sub_help(char **args, size_t args_len) {
    (void)args;
    (void)args_len;

    fprintf(stdout, "This is basic_sub_help()\n");
    return true;
}

bool basic_sub_a_fn(char **args, size_t args_len) {
    (void)args;
    (void)args_len;

    fprintf(stdout, "This is basic_sub_a_fn()\n");
    return true;
}

bool basic_sub_b_fn(char **args, size_t args_len) {
    (void)args;
    (void)args_len;

    fprintf(stdout, "This is basic_sub_b_fn()\n");
    return true;
}

void basic_command_info() {
    fprintf(stdout, "All Support commands: \n");
    fprintf(stdout, "\t help: \n");
    fprintf(stdout, "\t exit: \n");
    fprintf(stdout, "\t create: \n");
    fprintf(stdout, "\t use: \n \t\t <database name> \n");
    fprintf(stdout, "\t delete: \n \t\t <database name> \n");
    fprintf(stdout, "\t select: \n \t\t <column_names> from <table_name> (where <condition> ...) \n");
    fprintf(stdout, "\t insert: \n \t\t <table_name> values <value1,value2,value3,...> \n");
    fprintf(stdout, "\t list: \n");
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

void list_command_info(void) {
    fprintf(stdout, "List sub-commands: \n");
    fprintf(stdout, "\t database <database_name> \n");
    fprintf(stdout, "\t table <table_name> \n");
}

bool create_database_help_fn(char **args, size_t args_len) {
    (void)args;
    (void)args_len;

    create_command_info();
    return true;
}

bool create_database_fn(char **args, size_t args_len) {
    (void)args;
    (void)args_len;

    if (args_len != 2) {
        fprintf(stderr, "Wrong arguments\n");
        create_command_info();
        return false;
    }

    char db_foldername_full[PATH_MAX] = {0};
    snprintf(db_foldername_full, PATH_MAX, "%s/%s", WORKSPACE_PATH_FULL, args[1]);
    create_folder(db_foldername_full);

    // Create database file
    char db_filename_full[PATH_MAX] = {0};
    snprintf(db_filename_full, PATH_MAX, "%s/%s/%s.json", WORKSPACE_PATH_FULL, args[1], args[1]);
    create_database(db_filename_full);
    fprintf(stdout, "Create database at: %s \n", db_filename_full);

    return true;
}

void create_database(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to create database: %s \n", filename);
        DB_ASSERT(0);
    }
    fclose(file);

    create_database_meta(filename);
}

void create_database_meta(const char *filename) {
    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        DB_ASSERT(!"Failed to create JSON object \n");
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
        cJSON_Delete(json);
        DB_ASSERT(!"Failed to create tables array \n");
    }
    cJSON_AddItemToObject(json, "tables", tables_array);

    // Write to file
    char *json_str = cJSON_Print(json);
    if (json_str == NULL) {
        cJSON_Delete(json);
        DB_ASSERT(!"Failed to init cJSON_Print() \n");
    }

    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        fprintf(stderr, "Failed to write database: %s \n", filename);
        cJSON_free(json_str);
        cJSON_Delete(json);
        DB_ASSERT(0);
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
        DB_ASSERT(!"Failed to parse JSON\n");
    }

    // Get the tables array
    cJSON *tables = cJSON_GetObjectItem(json, "tables");
    if (!cJSON_IsArray(tables)) {
        cJSON_Delete(json);
        DB_ASSERT(!"\"table\" is not an array\n");
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
        fprintf(stderr, "Failed to open database file: %s\n", db_filename);
        cJSON_Delete(json);
        DB_ASSERT(0);
    }
    char *modified_content = cJSON_Print(json);
    fprintf(file, "%s", modified_content);
    fclose(file);
    cJSON_free(modified_content);
    cJSON_Delete(json);
}

bool delete_help_fn(char **args, size_t args_len) {
    (void)args;
    (void)args_len;

    delete_command_info();
    return true;
}

bool delete_database_fn(char **args, size_t args_len) {
    (void)args_len;

    // Delete database file
    char db_filename_full[PATH_MAX] = {0};
    snprintf(db_filename_full, PATH_MAX, "%s/%s/%s.json", WORKSPACE_PATH_FULL, args[1], args[1]);
    delete_database(db_filename_full);

    // Delete folder
    char db_foldername_full[PATH_MAX] = {0};
    snprintf(db_foldername_full, PATH_MAX, "%s/%s", WORKSPACE_PATH_FULL, args[1]);

    delete_table_all(db_foldername_full);
    remove_folder(db_foldername_full);

    // clean buffer
    clean_current_db();

    return true;
}

void delete_database(const char *filename) {
    if (remove(filename) != 0) {
        fprintf(stderr, "Failed to delete database: %s \n", filename);
    }
    else {
        fprintf(stdout, "Delete database: %s \n", filename);
    }
}

bool create_table_fn(char **args, size_t args_len) {
    const current_db_t *curr_db = get_current_db();
    if (curr_db->len == 0) {
        fprintf(stderr, "Error: Don't know what database to use, please use `USE` command to select database first \n");
        return false;
    }
    // Make sure command contain `create table table_name` and `column_name + type` pair
    else if ((args_len < 4) || ((args_len-2) % 2 != 0)) {
        fprintf(stderr, "Error: Argument not enough: (<column name> <column type> ...)");
        create_command_info();
        return false;
    }
    else {
        char table_filename_full[PATH_MAX] = {0};
        snprintf(table_filename_full, PATH_MAX, "%s/%s/%s.csv", WORKSPACE_PATH_FULL, curr_db->name, args[1]);
        create_table(table_filename_full, args[1], &args[2], args_len-2);
        fprintf(stdout, "Create table at: %s \n", table_filename_full);
    }

    return true;
}

void create_table(const char *filename_path, const char *filename, char **args, size_t len) {
    FILE *file = fopen(filename_path, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to create table: %s \n", filename_path);
        DB_ASSERT(0);
    }

    // Show column on the first line
    for (size_t i=0; i<(len-2); i+=2) {
        fprintf(file, "%s,", args[i]);
    }
    fprintf(file, "%s\n", args[len-2]);
    fclose(file);

    // Update new table to database meta data
    const current_db_t *curr_db = get_current_db();
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

bool use_help_fn(char **args, size_t args_len) {
    (void)args;
    (void)args_len;

    use_command_info();
    return true;
}

bool use_fn(char **args, size_t args_len) {
    (void)args_len;

    char db_folder_full[PATH_MAX] = {0};
    char db_filename_full[PATH_MAX] = {0};
    snprintf(db_folder_full, PATH_MAX, "%s/%s", WORKSPACE_PATH_FULL, args[0]);
    snprintf(db_filename_full, PATH_MAX, "%s/%s.json", db_folder_full, args[0]);

    if (!exist_file(db_filename_full)) {
        fprintf(stderr, "Database %s not exist \n", args[0]);
    }
    else {
        update_current_db(args[0], db_filename_full, db_folder_full);
        const current_db_t *db = get_current_db();
        fprintf(stdout, "Using database: %s \n", db->name);
    }

    return true;
}

bool insert_help_fn(char **args, size_t args_len) {
    (void)args;
    (void)args_len;

    insert_command_info();
    return true;
}

bool insert_fn(char **args, size_t args_len) {
    char table_name_path[PATH_MAX] = {0};
    if (check_table_exist((const char*)args[0], table_name_path)) {
        insert_table_data(table_name_path, (const char*)args[0], &args[2], args_len-2);
        return true;
    }
    else {
        fprintf(stderr, "Table %s not found\n", args[0]);
        return false;
    }
}

void insert_table_data(const char *filename_path, const char *table_name, char **datas, size_t len) {
    FILE *file = fopen(filename_path, "a");
    if (file == NULL) {
        fprintf(stderr, "Failed to insert data: %s \n", filename_path);
        DB_ASSERT(0);
    }

    for (size_t i=0; i<len-1; i++) {
        fprintf(file, "%s,", datas[i]);
    }
    fprintf(file, "%s\n", datas[len-1]);
    fclose(file);

    const current_db_t *curr_db = get_current_db();
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
        DB_ASSERT(!"Failed to parse JSON\n");
    }

    // find "tables" array
    cJSON *tables = cJSON_GetObjectItemCaseSensitive(root, "tables");
    if (!cJSON_IsArray(tables)) {
        cJSON_Delete(root);
        DB_ASSERT(!"\"tables\" is not an array\n");
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
        DB_ASSERT(0);
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
        DB_ASSERT(0);
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

bool select_help_fn(char **args, size_t args_len) {
    (void)args;
    (void)args_len;

    select_command_info();
    return true;
}

bool select_fn(char **args, size_t args_len) {
    (void)args_len;

    char table_name_path[PATH_MAX] = {0};
    char *table_name = args[2];
    delete_semicolon(table_name);
    // Process `from` command
    if (check_table_exist((const char*)table_name, table_name_path)) {
        // Load Table metadata
        table_data_t *table_data = select_load_table_metadata(table_name);

        // Parse SQL command
        size_t parsed_cmd_cnt = 0;
        prompt_buf_t *prompt = get_prompt_raw();
        parsed_sql_cmd_t *parsed_cmd = parse_sql_cmd((const char *)prompt->buf, &parsed_cmd_cnt);

        // Process `select` command
        bool column_found = select_fetch_available_column(table_data, &parsed_cmd[0]);

        // Process `where` command
        size_t condition_len = 0;
        where_args_cond_t conditions[WHERE_MATCH_CNT] = {0};
        bool row_status = select_fetch_available_row(table_data, &parsed_cmd[2], conditions, &condition_len);

        // Make sure there is no non-exist table column
        if  (column_found && row_status) {
            const current_db_t *db = get_current_db();
            snprintf(table_name_path, PATH_MAX, "%s/%s.csv", db->folder_path, table_name);
            select_load_table_data(table_data, table_name_path, conditions, condition_len);

            select_table_display(table_data);
        }
        else {
            fprintf(stderr, "Column name not found \n");
        }

        // Free
        select_table_close(table_data);
        parse_select_cmd_close(parsed_cmd);
        return true;
    }
    else {
        fprintf(stderr, "Table %s not found\n", args[2]);
        return false;
    }
}

void select_load_table_data(table_data_t *t, char *table_name_path, where_args_cond_t *conditions, size_t condition_len) {
    // Read table content
    u_int32_t res_lines = 0;
    char *table = read_file(table_name_path, 1, &res_lines);

    // Split into lines
    splitter_t lines_splitter = split_construct();
    size_t lines_num_tokens;
    char** lines = lines_splitter.run(table, "\n", &lines_num_tokens);

    // Split into cell, save to table_struct
    for (size_t i=0; i<lines_num_tokens; i++) {
        splitter_t cells_splitter = split_construct();
        size_t cells_num_tokens;
        char** cells = cells_splitter.run(lines[i], ",", &cells_num_tokens);

        bool not_skip = rpn_evaluate_where_conditions(t, cells, conditions, condition_len);
        if (!not_skip) {
            cells_splitter.free(cells, cells_num_tokens);
            continue;
        }

        // Filter out via selected column_name
        uint8_t tmp_row_idx = 0;
        char **tmp_row = (char**)calloc(t->col_enable_cnt,  sizeof(char*));
        for (size_t j=0; j<t->col_len; j++) {
            if (t->cols[j].enable) {
                uint8_t str_len = strlen(cells[j]) + 1;
                tmp_row[tmp_row_idx] = (char*)malloc(str_len * sizeof(char));
                strncpy(tmp_row[tmp_row_idx], cells[j], str_len);
                tmp_row_idx++;
            }
        }

        // insert data
        table_data_insert_row_data(t, tmp_row, tmp_row_idx);

        // Free
        for (size_t j=0; j<t->col_enable_cnt; j++) {
            free(tmp_row[j]);
        }
        free(tmp_row);
        cells_splitter.free(cells, cells_num_tokens);
    }

    // Close
    lines_splitter.free(lines, lines_num_tokens);
    free(table);
    table = NULL;
}

table_data_t* select_load_table_metadata(const char *table_name) {
    const current_db_t *db = get_current_db();

    uint32_t res_lines = 0;
    char *content = read_file(db->name_path, 0, &res_lines);

    // Parse the JSON content
    cJSON *root = cJSON_Parse(content);
    free(content);
    content = NULL;
    if (root == NULL) {
        DB_ASSERT(!"Failed to parse JSON\n");
    }

    // find "tables" array
    cJSON *tables = cJSON_GetObjectItemCaseSensitive(root, "tables");
    if (!cJSON_IsArray(tables)) {
        cJSON_Delete(root);
        DB_ASSERT(!"\"tables\" is not an array\n");
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
            column_len = cJSON_GetArraySize(columns);

            cJSON *rows = cJSON_GetObjectItemCaseSensitive(table, "data_cnt");
            if (!cJSON_IsNumber(rows)) {
                fprintf(stderr, "\"data_cnt\" is not a number for table %s\n", table_name);
                break;
            }
            size_t row_len = (size_t)cJSON_GetNumberValue(rows);

            // Create table info
            table_data = table_data_init(column_len, row_len);

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
            break;
        }
    }

    cJSON_Delete(root);
    return table_data;
}

bool select_fetch_available_column(table_data_t *t, parsed_sql_cmd_t *select_cmd) {
    if (select_cmd->state != SQL_SELECT_CMD) {
        DB_ASSERT(!"Wrong Command\n");
    }

    splitter_t col_splitter = split_construct();
    size_t col_num_tokens;
    char** column_names = col_splitter.run((const char*)select_cmd->args, ",", &col_num_tokens);

    // Find selected column_name
    for (size_t i = 0; i < col_num_tokens; i++) {
        bool found = false;
        for (size_t j = 0; j < t->col_len; j++) {
            if (strncmp(column_names[i], t->cols[j].name, strlen(t->cols[j].name)) == 0) {
                t->cols[j].enable = true;
                t->col_enable_cnt++;
                found = true;
                break;
            }
        }
        if (!found) {
            fprintf(stderr, "Column name not matched: %s to (",column_names[i] );
            for (size_t j = 0; j < t->col_len-1; j++) {
                fprintf(stderr, "%s, ", t->cols[j].name);
            }
            fprintf(stderr, "%s) \n", t->cols[t->col_len-1].name);

            col_splitter.free(column_names, col_num_tokens);
            return false;
        }
    }

    col_splitter.free(column_names, col_num_tokens);
    return true;
}

void select_parse_where_args(table_data_t *t, const char *sql_cmd, where_args_cond_t *conds, size_t *args_len) {
    regex_t regex;
    regmatch_t matches[2] = {0};
    int ret;
    size_t cond_idx = 0;

    // parse <column_name, operate, value> pair, logic operate, `(`, `)`
    const char *pattern = "(\\w+)\\s*([<>=]+)\\s*([[:digit:]]+\\.[[:digit:]]+|[[:digit:]]+|\\w+)|\\b(and|or)\\b|\\(|\\)";

    // regex compile
    ret = regcomp(&regex, pattern, REG_EXTENDED);
    if (ret) {
        DB_ASSERT(!"Could not compile regex\n");
    }

    // regex match
    char *cursor = (char*)sql_cmd;
    int sql_cmd_len = strlen(sql_cmd);
    char buf_str[CELL_TEXT_MAX] = {0};
    char val_str[CELL_TEXT_MAX] = {0};
    char op_str[5] = {0};
    size_t length;

    while (regexec(&regex, cursor, 1, matches, 0) == 0) {
        // matched chunk
        length = matches[0].rm_eo - matches[0].rm_so;
        strncpy(buf_str, cursor, length);
        buf_str[length] = '\0';

        // check `(`, `)`, `and`, `or`
        if (length > 0 && length <= 3) {
            logic_op_t tmp_op = calc_op_str(buf_str);
            if (is_op_parenthesis(tmp_op) || is_op_and_or(tmp_op)) {
                conds[cond_idx].op = tmp_op;
                cond_idx++;
            }
        }
        // check <column_name, operate, value>
        else {
            sscanf(buf_str, "%19s %4s %31s", conds[cond_idx].column, op_str, val_str);
            for (size_t i=0; i<t->col_len; i++) {
                if (calc_val_str(t, conds, cond_idx, i, op_str, val_str)) {
                    break;
                }
            }
            cond_idx++;
        }

        if (matches[0].rm_eo + 1 > sql_cmd_len) {
            break;
        }

        cursor += matches[0].rm_eo + 1;
        DB_ASSERT(cond_idx < WHERE_MATCH_CNT && "Out of range\n");
    }

    regfree(&regex);
    *args_len = cond_idx;
}

// `where` format should be like below
// One condition: <column_name>,<op>,<value>
// Multi-condition: <column_name>,<op>,<value> <and/or> <column_name>,<op>,<value>
bool select_fetch_available_row(table_data_t *t, parsed_sql_cmd_t *select_cmd, where_args_cond_t *conditions, size_t *condition_len) {
    if (select_cmd->state != SQL_WHERE_CMD) {
        DB_ASSERT(!"Wrong Command\n");
    }

    // Parse `where` args
    where_args_cond_t infix_conditions[WHERE_MATCH_CNT] = {0};
    select_parse_where_args(t, (const char *)select_cmd->args, infix_conditions, condition_len);

    // run Reverse Polish Notation (RPN)
    rpn_infix_to_postfix(infix_conditions, conditions, *condition_len);

    // validate `where` column_name
    for (size_t i=0; i<(*condition_len); i++) {
        if (!is_op_and_or(infix_conditions[i].op) && !is_op_parenthesis(infix_conditions[i].op)) {
            bool found = false;
            for (size_t j=0; j<t->col_len; j++) {
                if (t->cols[j].enable && compare_column_name(t->cols[j].name, infix_conditions[i].column)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return false;
            }
        }
    }

    return true;
}

void select_table_display(table_data_t *t) {
    size_t len = t->col_enable_cnt;

    // Print column names
    for (size_t i=0; i<len-1; i++) {
        fprintf(stdout, "%s,", t->cols[i].name);
    }
    fprintf(stdout, "%s\n", t->cols[len-1].name);

    // Print data
    table_row_t *head = t->rows;
    while (head) {
        for (size_t i=0; i<len-1; i++) {
            fprintf(stdout, "%s,", head->data[i].cell);
        }
        fprintf(stdout, "%s", head->data[len-1].cell);
        fprintf(stdout, "\n");

        head = head->next;
    }
}

void select_table_close(table_data_t *t) {
    table_data_close(t);
}

bool check_table_exist(const char *table_name, char *table_name_path) {
    const current_db_t *db = get_current_db();
    snprintf(table_name_path, PATH_MAX, "%s/%s.csv", db->folder_path, table_name);
    return exist_file(table_name_path);
}

bool compare_column_name(const char *ref, const char *src) {
    size_t str_len = strlen(ref);
    return strncmp(ref, src, str_len) == 0;
}

size_t find_column_name_idx(table_data_t *t, const char *col_name) {
    size_t str_len = strlen(col_name);
    for (size_t i=0; i<str_len; i++) {
        if (strncmp(t->cols[i].name, col_name, str_len) == 0) {
            return i;
        }
    }
    return -1;
}

bool wrong_fn(char **args, size_t args_len) {
    (void)args;
    (void)args_len;

    fprintf(stderr, "Wrong Command\n");
    return false;
}

bool null_fn(char **args, size_t args_len) {
    (void)args;
    (void)args_len;

    DB_ASSERT(!"No Implementation.\n");
    return false;
}

bool undefined_fn(char **args, size_t args_len) {
    fprintf(stderr, "Unrecognized command\n\n");
    basic_fn(args, args_len);
    return true;
}

bool exit_fn(char **args, size_t args_len) {
    (void)args;
    (void)args_len;

    fprintf(stdout, "Bye Bye \n");
    return true;
}