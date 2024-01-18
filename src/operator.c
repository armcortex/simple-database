//
// Created by MCS51_M2 on 2024/1/18.
//

#include "operator.h"
#include "string.h"

#include "helper_functions.h"

bool is_op_null(logic_op_t op) {
    return op == OP_NULL;
}

bool is_op_and_or(logic_op_t op) {
    return (op == OP_AND) || (op == OP_OR);
}

bool is_op_parenthesis(logic_op_t op) {
    return (op == OP_OPEN_PARENTHESIS) || (op == OP_CLOSE_PARENTHESIS);
}

bool is_operator(logic_op_t op) {
    return is_op_and_or(op);
}

bool is_operand(logic_op_t op) {
    return (op == OP_EQ) || (op == OP_NE) || (op == OP_LT) || (op == OP_GT) || (op == OP_LE) || (op == OP_GE);
}

logic_op_t calc_op_str(const char *op) {
    if (strncmp(op, "and", 3) == 0) {
        return OP_AND;
    }
    else if (strncmp(op, "or", 2) == 0) {
        return OP_OR;
    }
    else if (strncmp(op, "!=", 2) == 0) {
        return OP_NE;
    }
    else if (strncmp(op, "<=", 2) == 0) {
        return OP_LE;
    }
    else if (strncmp(op, ">=", 2) == 0) {
        return OP_GE;
    }
    else if (strncmp(op, "=", 1) == 0) {
        return OP_EQ;
    }
    else if (strncmp(op, "<", 1) == 0) {
        return OP_LT;
    }
    else if (strncmp(op, ">", 1) == 0) {
        return OP_GT;
    }
    else if (strncmp(op, "(", 1) == 0) {
        return OP_OPEN_PARENTHESIS;
    }
    else if (strncmp(op, ")", 1) == 0) {
        return OP_CLOSE_PARENTHESIS;
    }
    else {
        fprintf(stderr, "Command not supported: %s\n", op);
        DB_ASSERT(0);
    }
}

void calc_val_str(table_data_t *t, where_args_cond_t *cond, size_t cond_idx, size_t col_idx, char *op_str, char *val_str) {
    uint8_t str_len = strlen(t->cols[col_idx].name);
    if (t->cols[col_idx].enable && strncmp(cond[cond_idx].column, t->cols[col_idx].name, str_len)==0) {
        cond[cond_idx].op = calc_op_str(op_str);

        switch (t->cols[col_idx].type) {
            case TABLE_STRING: {
                strncpy(cond[cond_idx].val.s, val_str, strlen(val_str));
            }
                break;
            case TABLE_INT: {
                cond[cond_idx].val.i = atoi(val_str);
            }
                break;
            case TABLE_FLOAT: {
                cond[cond_idx].val.f = atof(val_str);
            }
                break;
            default: {
                DB_ASSERT(!"Unsupported column type\n");
            }
        }
    }
}