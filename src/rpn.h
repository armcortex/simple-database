//
// Created by MCS51_M2 on 2024/1/4.
//

#ifndef SIMPLE_DATABASE_RPN_H
#define SIMPLE_DATABASE_RPN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>

#include "cmd_functions.h"

#define RPN_STACK_MAX           (10)

typedef struct rpn_stack_t {
    where_args_cond_t items[RPN_STACK_MAX];
    ssize_t top;

    void (*init)(struct rpn_stack_t *ctx);
    void (*free)(struct rpn_stack_t *ctx);
    bool (*push)(struct rpn_stack_t *ctx, where_args_cond_t item);
    where_args_cond_t (*pop)(struct rpn_stack_t *ctx);
    bool (*is_empty)(struct rpn_stack_t *ctx);
    bool (*is_full)(struct rpn_stack_t *ctx);
} rpn_stack_t;

rpn_stack_t rpn_stack_construct(void);

void infix_to_postfix(where_args_cond_t *infix, where_args_cond_t *postfix, size_t len);

bool evaluate_where_conditions(table_data_t *t, char **cell, size_t cell_len, where_args_cond_t *conditions, size_t condition_len);

int main_rpn(void);

#ifdef __cplusplus
}
#endif


#endif //SIMPLE_DATABASE_RPN_H
