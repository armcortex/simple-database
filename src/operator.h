//
// Created by MCS51_M2 on 2024/1/18.
//

#ifndef SIMPLE_DATABASE_OPERATOR_H
#define SIMPLE_DATABASE_OPERATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "table.h"

bool is_op_null(logic_op_t op);
bool is_op_and_or(logic_op_t op);
bool is_op_parenthesis(logic_op_t op);
bool is_operator(logic_op_t op);
bool is_operand(logic_op_t op);

logic_op_t calc_op_str(const char *op);
void calc_val_str(table_data_t *t, where_args_cond_t *cond, size_t cond_idx, size_t col_idx, char *op_str, char *val_str);

#ifdef __cplusplus
}
#endif

#endif //SIMPLE_DATABASE_OPERATOR_H
