//
// Created by MCS51_M2 on 2023/12/8.
//

#include "parser.h"
#include "helper_functions.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <regex.h>


select_parsed_data_t* parse_select_cmd(const char *sql_cmd, size_t *match_cnt) {
    select_parsed_data_t *parsed_data = NULL;
    regex_t regex;
    regmatch_t matches[MAX_MATCHES] = {0};
    int ret;

    // Get `select`, `from`, and `where` their arguments
    char *pattern = "select[[:space:]]+([[:alnum:]_, ]+)[[:space:]]+from[[:space:]]+([[:alnum:]_, ]+)[[:space:]]+where[[:space:]]+([^;]+))?";

    // compile regex
    ret = regcomp(&regex, pattern, REG_EXTENDED);
    if (ret) {
        fprintf(stderr, "Could not compile regex\n");
        assert(0);
    }

    // regex match
    ret = regexec(&regex, sql_cmd, MAX_MATCHES, matches, 0);
    if (!ret) {
        parsed_data = (select_parsed_data_t *)calloc(MAX_MATCHES,  sizeof(select_parsed_data_t));
        for (int i=1; i<MAX_MATCHES; i++) {
            if (matches[i].rm_so != (size_t)-1) {
                size_t args_len = matches[i].rm_eo - matches[i].rm_so;
                parsed_data[i-1].state = (select_state_t)(i-1);
                parsed_data[i-1].args = (char*)malloc(args_len * sizeof(char));
                strncpy(parsed_data[i-1].args, sql_cmd + matches[i].rm_so, args_len);
                parsed_data[i-1].args[args_len] = '\0';
                logger_str(true, "Match %d: %s\n", i, parsed_data[i-1].args);
                *match_cnt += 1;
            }
        }
    }
    else if (ret == REG_NOMATCH) {
        fprintf(stderr, "No match found\n");
        logger_str(true, "No match found\n");
        assert(0);
    }
    else {
        fprintf(stderr, "Regex match failed\n");
        logger_str(true, "Regex match failed\n");
        assert(0);
    }

    // free
    regfree(&regex);

    return parsed_data;
}

void parse_select_cmd_close(select_parsed_data_t *parsed_data) {
    if (parsed_data != NULL) {
        for (size_t i=0; i<MAX_MATCHES-1; i++) {
            if (parsed_data[i].args != NULL ) {
                free(parsed_data[i].args);
                parsed_data[i].args = NULL;
            }
        }
        free(parsed_data);
        parsed_data = NULL;
    }
}

const char* check_select_column_names_correct(select_parsed_data_t *parsed_data, table_data_t *table_data) {
    if (parsed_data[0].state != SELECT_SELECT_CMD) {
        fprintf(stderr, "Wrong Command\n");
        assert(0);
    }

    splitter_t col_splitter = split_construct();
    size_t col_num_tokens;
    char** column_names = col_splitter.run((const char*)parsed_data[0].args, ",", &col_num_tokens);
    for (size_t i=0; i<col_num_tokens; i++) {
        if (strncmp(column_names[i], table_data->table_column_names[i], strlen(table_data->table_column_names[i])) != 0) {
            fprintf(stderr, "Column name not matched: %s vs %s \n", table_data->table_column_names[i], column_names[i]);
            return column_names[i];
        }
        else {
            logger_str(true, "column name: %s vs %s\n", table_data->table_column_names[i], column_names[i] );
        }
    }

    col_splitter.free(column_names, col_num_tokens);
    return "\0";
}
