//
// Created by MCS51_M2 on 2024/1/4.
//

// Reverse Polish Notation (RPN)

#include "rpn.h"
#include "helper_functions.h"
#include <stdlib.h>
#include <string.h>

static void rpn_init(rpn_stack_t *ctx) {
    ctx->top = -1;
}

static void rpn_free(rpn_stack_t *ctx) {
    ;
}

static size_t rpn_count(rpn_stack_t *ctx) {
    return ctx->top;
}

static bool rpn_is_empty(rpn_stack_t *ctx) {
    return ctx->top == -1;
}

static bool rpn_is_full(rpn_stack_t *ctx) {
    return ctx->top == RPN_STACK_MAX;
}

static bool rpn_push(rpn_stack_t *ctx, where_args_cond_t item) {
    if (rpn_is_full(ctx)) {
        fprintf(stderr, "RPN stack is full\n");
        return false;
    }

    ctx->items[++ctx->top] = item;
    return true;
}

static where_args_cond_t rpn_pop(rpn_stack_t *ctx) {
    if (rpn_is_empty(ctx)) {
        fprintf(stderr, "RPN stack is empty\n");
        where_args_cond_t null = {0};
        return null;
    }
    return ctx->items[ctx->top--];
}

logic_op_t rpn_peek_op(rpn_stack_t *ctx, size_t idx) {
    return ctx->items[idx].op;
}

rpn_stack_t rpn_stack_construct() {
    rpn_stack_t ctx = {
        .top = -1,
        .init = rpn_init,
        .free = rpn_free,
        .push = rpn_push,
        .pop = rpn_pop,
        .is_empty = rpn_is_empty,
        .is_full = rpn_is_full
    };
    memset(ctx.items, 0, RPN_STACK_MAX * sizeof(where_args_cond_t));
    return ctx;
}

void infix_to_postfix(where_args_cond_t *infix, where_args_cond_t *postfix, size_t len) {
    rpn_stack_t stack = rpn_stack_construct();
    size_t j=0;
    for (size_t i=0; i<len; i++) {
        if (is_op_and_or(infix[i].op)) {
            while (!rpn_is_empty(&stack)) {
                postfix[j++] = rpn_pop(&stack);
            }
            rpn_push(&stack, infix[i]);
        }
        else {
            postfix[j++] = infix[i];
        }
    }

    while (!rpn_is_empty(&stack)) {
        postfix[j++] = rpn_pop(&stack);
    }
}

bool calc_condition(table_data_t *t, char **cell,size_t col_idx ,where_args_cond_t *conditions, logic_op_t op) {
    switch (op) {
        case OP_EQ: {
            switch (t->cols[col_idx].type) {
                case TABLE_STRING: {
                    if (!compare_column_name((const char*)cell[col_idx], (const char*)conditions->val.s)) {
                        return false;
                    }
                }
                break;
                case TABLE_INT: {
                    if (atoi(cell[col_idx]) != conditions->val.i) {
                        return false;
                    }
                }
                break;
                case TABLE_FLOAT: {
                    if (atof(cell[col_idx]) != conditions->val.f) {
                        return false;
                    }
                }
                break;
                default: {
                    DB_ASSERT(!"Unsupported operator\n");
                }
            }
        }
        break;
        case OP_NE: {
            switch (t->cols[col_idx].type) {
                case TABLE_STRING: {
                    if (compare_column_name((const char*)cell[col_idx], (const char*)conditions->val.s)) {
                        return false;
                    }
                }
                break;
                case TABLE_INT: {
                    if (atoi(cell[col_idx]) == conditions->val.i) {
                        return false;
                    }
                }
                break;
                case TABLE_FLOAT: {
                    if (atof(cell[col_idx]) == conditions->val.f) {
                        return false;
                    }
                }
                break;
                default: {
                    DB_ASSERT(!"Unsupported operator\n");
                }
            }
        }
        break;
        case OP_LT: {
            switch (t->cols[col_idx].type) {
                case TABLE_INT: {
                    if (atoi(cell[col_idx]) >= conditions->val.i) {
                        return false;
                    }
                }
                break;
                case TABLE_FLOAT: {
                    if (atof(cell[col_idx]) >= conditions->val.f) {
                        return false;
                    }
                }
                break;
                default: {
                    DB_ASSERT(!"Unsupported operator\n");
                }
            }
        }
        break;
        case OP_GT: {
            switch (t->cols[col_idx].type) {
                case TABLE_INT: {
                    if (atoi(cell[col_idx]) <= conditions->val.i) {
                        return false;
                    }
                }
                break;
                case TABLE_FLOAT: {
                    if (atof(cell[col_idx]) <= conditions->val.f) {
                        return false;
                    }
                }
                break;
                default: {
                    DB_ASSERT(!"Unsupported operator\n");
                }
            }
        }
        break;
        case OP_LE: {
            switch (t->cols[col_idx].type) {
                case TABLE_INT: {
                    if (atoi(cell[col_idx]) > conditions->val.i) {
                        return false;
                    }
                }
                break;
                case TABLE_FLOAT: {
                    if (atof(cell[col_idx]) > conditions->val.f) {
                        return false;
                    }
                }
                break;
                default: {
                    DB_ASSERT(!"Unsupported operator\n");
                }
            }
        }
        break;
        case OP_GE: {
            switch (t->cols[col_idx].type) {
                case TABLE_INT: {
                    if (atoi(cell[col_idx]) < conditions->val.i) {
                        return false;
                    }
                }
                break;
                case TABLE_FLOAT: {
                    if (atof(cell[col_idx]) < conditions->val.f) {
                        return false;
                    }
                }
                break;
                default: {
                    DB_ASSERT(!"Unsupported operator\n");
                }
            }
        }
        break;
        default: {
            DB_ASSERT(!"Unsupported operator\n");
        }
    }
    return true;
}

bool evaluate_where_conditions(table_data_t *t, char **cell, size_t cell_len, where_args_cond_t *conditions, size_t condition_len) {
    rpn_stack_t stack = rpn_stack_construct();

    for (size_t i=0; i < condition_len; i++) {
        // Process op OP_AND, OP_OR
        if (!is_op_and_or(conditions[i].op)) {
            where_args_cond_t x1 = conditions[i];
            size_t x1_idx = find_column_name_idx(t, x1.column);
            bool x1_succeed = calc_condition(t, cell, x1_idx, &x1, x1.op);
            if (!x1_succeed)   return false;

            rpn_push(&stack, conditions[i]);
        }
        // Process other logic calculate
        else {
            if (rpn_count(&stack) < 1) {
                DB_ASSERT(!"Operator should be 2\n");
            }

            // char s[] = "age < 29 and name = Jane or name = Alice";
            where_args_cond_t x1 = rpn_pop(&stack);
            where_args_cond_t x2 = rpn_pop(&stack);
            switch (conditions[i].op) {
                case OP_AND: {
                    size_t x1_idx = find_column_name_idx(t, x1.column);
                    bool x1_succeed = calc_condition(t, cell, x1_idx, &x1, x1.op);
                    if (!x1_succeed)   return false;

                    size_t x2_idx = find_column_name_idx(t, x2.column);
                    bool x2_succeed = calc_condition(t, cell, x2_idx, &x2, x2.op);
                    if (!x2_succeed)   return false;
                }
                break;
                case OP_OR: {
                    size_t x1_idx = find_column_name_idx(t, x1.column);
                    bool x1_succeed = calc_condition(t, cell, x1_idx, &x1, x1.op);

                    size_t x2_idx = find_column_name_idx(t, x2.column);
                    bool x2_succeed = calc_condition(t, cell, x2_idx, &x2, x2.op);
                    if ((!x1_succeed) && (!x2_succeed))   return false;
                }
                break;
                default: {
                    DB_ASSERT(!"Unsupported operator\n");
                }
            }
            rpn_push(&stack, x2);
        }
    }
    return true;
}

int main_rpn() {

    return 0;
}
