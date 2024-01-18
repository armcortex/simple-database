//
// Created by MCS51_M2 on 2024/1/4.
//

// Reverse Polish Notation (RPN)

#include <stdlib.h>
#include <string.h>

#include "rpn.h"
#include "helper_functions.h"
#include "table.h"
#include "operator.h"
#include "cmd_functions.h"

static void rpn_init(rpn_stack_t *ctx) {
    ctx->top = -1;
    ctx->items = (where_args_cond_t*)calloc(RPN_STACK_MAX, sizeof(where_args_cond_t));
    if (ctx->items == NULL) {
        DB_ASSERT(!"Failed to allocate memory.\n");
    }
}

static void rpn_free(rpn_stack_t *ctx) {
    free(ctx->items);
}

static ssize_t rpn_get_index(rpn_stack_t *ctx) {
    return ctx->top;
}

static bool rpn_is_empty(rpn_stack_t *ctx) {
    return ctx->top == -1;
}

static bool rpn_is_full(rpn_stack_t *ctx) {
    return ctx->top == RPN_STACK_MAX;
}

static int8_t rpn_priority(logic_op_t op) {
    if (op == OP_AND || op == OP_OR) {
        return 0;
    }
    else {
        return -1;
    }
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

static logic_op_t rpn_get_top_op(rpn_stack_t *ctx) {
    return ctx->items[ctx->top].op;
}

rpn_stack_t rpn_stack_construct() {
    rpn_stack_t ctx = {
        .init = rpn_init,
        .free = rpn_free,
        .push = rpn_push,
        .pop = rpn_pop,
        .priority = rpn_priority,
        .get_top_op = rpn_get_top_op,
        .is_empty = rpn_is_empty,
        .is_full = rpn_is_full,
        .get_index = rpn_get_index,
    };

    ctx.init(&ctx);
    return ctx;
}

void rpn_infix_to_postfix(where_args_cond_t *infix, where_args_cond_t *postfix, size_t len) {
    rpn_stack_t stack = rpn_stack_construct();

    size_t j = 0;
    for (size_t i=0; i<len; i++) {
        if (is_op_null(infix[i].op)) {
            continue;
        }

        if (is_operand(infix[i].op)) {
            postfix[j++] = infix[i];
        }
        else if (is_op_parenthesis(infix[i].op)) {
            if (infix[i].op == OP_OPEN_PARENTHESIS) {
                stack.push(&stack, infix[i]);
            }
            else {
                while ((!stack.is_empty(&stack)) && (stack.get_top_op(&stack) != OP_OPEN_PARENTHESIS)) {
                    postfix[j++] = stack.pop(&stack);
                }
                stack.pop(&stack);
            }
        }
        else if (is_operator(infix[i].op)) {
            while ((!stack.is_empty(&stack)) && (stack.priority(infix[i].op) <= stack.priority(stack.get_top_op(&stack)))) {
                postfix[j++] = stack.pop(&stack);
            }
            stack.push(&stack, infix[i]);
        }
        else {
            stack.free(&stack);
            DB_ASSERT(!"Unsupported operator\n");
        }
    }

    // pop all remain elements
    while (!stack.is_empty(&stack)) {
        postfix[j++] = stack.pop(&stack);
    }

    stack.free(&stack);
}

static bool rpn_check_string_condition(const char* cell_value, const char* condition_value, logic_op_t op) {
    bool cmp = compare_column_name(cell_value, condition_value);
    switch (op) {
        case OP_EQ: return cmp == true;
        case OP_NE: return cmp != true;
        default: DB_ASSERT(!"Unsupported operator\n");
    }
}

static bool rpn_check_int_condition(const char* cell_value, int condition_value, logic_op_t op) {
    int value = atoi(cell_value);
    switch (op) {
        case OP_EQ: return value == condition_value;
        case OP_NE: return value != condition_value;
        case OP_LT: return value < condition_value;
        case OP_GT: return value > condition_value;
        case OP_LE: return value <= condition_value;
        case OP_GE: return value >= condition_value;
        default: DB_ASSERT(!"Unsupported operator\n");
    }
}

static bool rpn_check_float_condition(const char* cell_value, float condition_value, logic_op_t op) {
    float value = atof(cell_value);
    switch (op) {
        case OP_EQ: return value == condition_value;
        case OP_NE: return value != condition_value;
        case OP_LT: return value < condition_value;
        case OP_GT: return value > condition_value;
        case OP_LE: return value <= condition_value;
        case OP_GE: return value >= condition_value;
        default: DB_ASSERT(!"Unsupported operator\n");
    }
}

static bool rpn_calc_condition(table_data_t *t, char **cell, size_t col_idx, where_args_cond_t *conditions, logic_op_t op) {
    table_col_type_t type = t->cols[col_idx].type;
    switch (type) {
        case TABLE_STRING:
            return rpn_check_string_condition(cell[col_idx], conditions->val.s, op);
        case TABLE_INT:
            return rpn_check_int_condition(cell[col_idx], conditions->val.i, op);
        case TABLE_FLOAT:
            return rpn_check_float_condition(cell[col_idx], conditions->val.f, op);
        default:
            DB_ASSERT(!"Unsupported column type\n");
    }
}

bool rpn_evaluate_where_conditions(table_data_t *t, char **cell, size_t cell_len, where_args_cond_t *conditions, size_t condition_len) {
    rpn_stack_t stack = rpn_stack_construct();

    for (size_t i=0; i < condition_len; i++) {
        if (is_op_null(conditions[i].op)) {
            continue;
        }

        // Process operand
        if (is_operand(conditions[i].op)) {
            stack.push(&stack, conditions[i]);
        }
        // Process other logic calculate
        else {
            if (stack.get_index(&stack) < 1) {
                stack.free(&stack);
                DB_ASSERT(!"Operator should be 2\n");
            }

            where_args_cond_t x1 = stack.pop(&stack);
            where_args_cond_t x2 = stack.pop(&stack);
            where_args_cond_t tmp = {0};
            switch (conditions[i].op) {
                case OP_AND: {
                    size_t x1_idx = find_column_name_idx(t, x1.column);
                    bool x1_succeed = rpn_calc_condition(t, cell, x1_idx, &x1, x1.op);
                    if (!x1_succeed) {
                        stack.free(&stack);
                        return false;
                    }

                    size_t x2_idx = find_column_name_idx(t, x2.column);
                    bool x2_succeed = rpn_calc_condition(t, cell, x2_idx, &x2, x2.op);
                    if (!x2_succeed) {
                        stack.free(&stack);
                        return false;
                    }
                    tmp = x2;
                }
                break;
                case OP_OR: {
                    size_t x1_idx = find_column_name_idx(t, x1.column);
                    bool x1_succeed = rpn_calc_condition(t, cell, x1_idx, &x1, x1.op);

                    size_t x2_idx = find_column_name_idx(t, x2.column);
                    bool x2_succeed = rpn_calc_condition(t, cell, x2_idx, &x2, x2.op);

                    if ((x1_succeed == false) && (x2_succeed == false)) {
                        stack.free(&stack);
                        return false;
                    }

                    if (x1_succeed) tmp = x1;
                    if (x2_succeed) tmp = x2;
                }
                break;
                default: {
                    stack.free(&stack);
                    DB_ASSERT(!"Unsupported operator\n");
                }
            }
            stack.push(&stack, tmp);
        }
    }

    // check remain elements
    while (!stack.is_empty(&stack)) {
        where_args_cond_t x1 = stack.pop(&stack);
        size_t x1_idx = find_column_name_idx(t, x1.column);
        bool x1_succeed = rpn_calc_condition(t, cell, x1_idx, &x1, x1.op);
        if (!x1_succeed) {
            stack.free(&stack);
            return false;
        }
    }

    stack.free(&stack);
    return true;
}
