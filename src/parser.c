//
// Created by MCS51_M2 on 2023/12/8.
//

#include "parser.h"
#include "helper_functions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>


parsed_sql_cmd_t* parse_sql_cmd(const char *sql_cmd, size_t *match_cnt) {
    parsed_sql_cmd_t *parsed_data = NULL;
    regex_t regex;
    regmatch_t matches[MAX_MATCHES] = {0};
    int ret;

    // Get `select`, `from`, and `where` their arguments
    char *pattern = "select[[:space:]]+([[:alnum:]_, ]+)[[:space:]]+from[[:space:]]+([[:alnum:]_, ]+)[[:space:]]+where[[:space:]]+([^;]+))?";

    // compile regex
    ret = regcomp(&regex, pattern, REG_EXTENDED);
    if (ret) {
        DB_ASSERT(!"Could not compile regex\n");
    }

    // regex match
    ret = regexec(&regex, sql_cmd, MAX_MATCHES, matches, 0);
    if (!ret) {
        parsed_data = (parsed_sql_cmd_t *)calloc(MAX_MATCHES, sizeof(parsed_sql_cmd_t));
        for (int i=1; i<MAX_MATCHES; i++) {
            if (matches[i].rm_so != (int)-1) {
                size_t args_len = matches[i].rm_eo - matches[i].rm_so + 1;
                parsed_data[i-1].state = (sql_state_t)(i-1);
                parsed_data[i-1].args = (char*)malloc(args_len * sizeof(char));
                strncpy(parsed_data[i-1].args, sql_cmd + matches[i].rm_so, args_len);
                parsed_data[i-1].args[args_len-1] = '\0';

                logger_str(true, "Match %d: %s\n", i, parsed_data[i-1].args);
                *match_cnt += 1;
            }
        }
    }
    else if (ret == REG_NOMATCH) {
        logger_str(true, "No match found\n");
        DB_ASSERT(!"No match found\n");
    }
    else {
        logger_str(true, "Regex match failed\n");
        DB_ASSERT(!"Regex match failed\n");
    }

    // free
    regfree(&regex);

    return parsed_data;
}

void parse_select_cmd_close(parsed_sql_cmd_t *parsed_data) {
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
