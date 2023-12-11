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


void parse_select_cmd(const char *sql_cmd, select_parsed_data_t **parsed_data) {
    regex_t regex;
    regmatch_t matches[MAX_MATCHES] = {0};
    int ret;

    // Get `select`, `from`, and `where` their arguments
    char *pattern = "select[[:space:]]+([[:alnum:]_, ]+)[[:space:]]+from[[:space:]]+([[:alnum:]_, ]+)[[:space:]]+where[[:space:]]+([^;]+))?";

    // Compile regex
    ret = regcomp(&regex, pattern, REG_EXTENDED);
    if (ret) {
        fprintf(stderr, "Could not compile regex\n");
        assert(0);
    }

    *parsed_data = (select_parsed_data_t *)malloc(MAX_MATCHES * sizeof(select_parsed_data_t));


    // regex match
    ret = regexec(&regex, sql_cmd, MAX_MATCHES, matches, 0);
    if (!ret) {
        for (int i=1; i<MAX_MATCHES; i++) {
            if (matches[i].rm_so != (size_t)-1) {
                size_t args_len = matches[i].rm_eo - matches[i].rm_so;
                (*parsed_data)[i-1].state = (select_state_t)(i-1);
                (*parsed_data)[i-1].args = (char*)malloc(args_len * sizeof(char));
                strncpy((*parsed_data)[i-1].args, sql_cmd + matches[i].rm_so, args_len);
                (*parsed_data)[i-1].args[args_len] = '\0';
                logger_str(true, "Match %d: %s\n", i, (*parsed_data)[i-1].args);
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
}

void parse_select_cmd_close(select_parsed_data_t **parsed_data) {
    if (*parsed_data != NULL) {
        for (size_t i=0; i<MAX_MATCHES-1; i++) {
            if ((*parsed_data)[i].args != NULL ) {
                free((*parsed_data)[i].args);
                (*parsed_data)[i].args = NULL;
            }
        }
        free(*parsed_data);
        *parsed_data = NULL;
    }
}
