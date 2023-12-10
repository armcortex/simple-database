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


void parse_select_cmd(const char *sql_cmd, select_parsed_data_t *parsed_data) {
#if 0
    {
        int match;
        int err;
        regex_t preg;
        regmatch_t pmatch[4]; // We have 3 capturing groups + the whole match group
        const char *str_regex = "([A-Za-z]+) +(http?://.*) +(HTTP/[0-9][.][0-9])";
        const char *str_request = "GET http://www.example.com HTTP/1.1";

        err = regcomp(&preg, str_regex, REG_EXTENDED);
        if (err) {
            printf("Could not compile regex\n");
            assert(0);
        }

        match = regexec(&preg, str_request, 4, pmatch, 0);
        if (match) {
            printf("No match\n");
        } else {
            printf("Match\n");
        }

        regfree(&preg);
    }
#endif


    regex_t regex;
    regmatch_t matches[MAX_MATCHES] = {0};
    int ret;

//    char *pattern = "select\\s+([\\w_, ]+)\\s+from\\s+(\\w+)(\\s+where\\s+([^;]+))?";
//    char *pattern = "select[[:space:]]+([[:alnum:]_, ]+)[[:space:]]+from[[:space:]]+([[:alnum:]_]+)([[:space:]]+where[[:space:]]+([^;]*))?";
//
    char *pattern1 = "select[[:space:]]+([[:alnum:]_, ]+)[[:space:]]+from[[:space:]]+([[:alnum:]_]+)(?:[[:space:]]+where[[:space:]]+([^;]*))?";
    char *pattern2 = "select[[:space:]]+([[:alnum:]_, ]+)[[:space:]]+from[[:space:]]+([[:alnum:]_]+)([[:space:]]+where[[:space:]]+)?([^;]*)";
    char *pattern3 = "select[[:space:]]+([[:alnum:]_, ]+)[[:space:]]+from[[:space:]]+([[:alnum:]_]+)([[:space:]]+where[[:space:]]+)?([^;]+)";

    char *select_str = "select[[:space:]]+([[:alnum:]_, ]+)";
    char *from_str   = "from[[:space:]]+([[:alnum:]_, ]+)";
    char *where_str  = "where[[:space:]]+([^;]+))?";

    char *all_str = "select[[:space:]]+([[:alnum:]_, ]+)[[:space:]]+from[[:space:]]+([[:alnum:]_, ]+)[[:space:]]+where[[:space:]]+([^;]+))?";

    char *pattern = all_str;
//    char pattern[500] = {0};
//    snprintf(pattern, 500, "%s[[:space:]]+%s[[:space:]]+%s", select_str, from_str, where_str);



    // Compile regex
    ret = regcomp(&regex, pattern, REG_EXTENDED);
    if (ret) {
        fprintf(stderr, "Could not compile regex\n");
        assert(0);
    }

    parsed_data = (select_parsed_data_t *)malloc(MAX_MATCHES * sizeof(select_parsed_data_t));


    // regex match
    ret = regexec(&regex, sql_cmd, MAX_MATCHES, matches, 0);
    if (!ret) {
        for (int i = 1; i < MAX_MATCHES; i++) {
            if (matches[i].rm_so != (size_t)-1) {
                char matched[100] = {0};
                strncpy(matched, sql_cmd + matches[i].rm_so, matches[i].rm_eo - matches[i].rm_so);
                matched[matches[i].rm_eo - matches[i].rm_so] = '\0';
                fprintf(stdout, "Match %d: %s\n", i, matched);
                logger_str(true, "Match %d: %s\n", i, matched);
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
    free(parsed_data);
    regfree(&regex);
}
