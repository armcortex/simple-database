//
// Created by MCS51_M2 on 2024/1/6.
//
#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include <vector>

#include "../src/cmd_functions.h"
#include "../src/rpn.h"


bool compare_where_args_cond_t(const where_args_cond_t& a, const where_args_cond_t& b) {
    return memcmp(&a.column, &b.column, sizeof(a.column)) == 0 &&
           memcmp(&a.op, &b.op, sizeof(a.op)) == 0 &&
           memcmp(&a.val, &b.val, sizeof(a.val)) == 0;
}

bool compare_batch_where_args_cond_t(const where_args_cond_t a[], const where_args_cond_t b[], size_t len) {
    for (size_t i=0; i<len; i++) {
        if (!compare_where_args_cond_t(a[i], b[i]))   return false;
    }
    return true;
}

TEST_CASE("RPN infix/postfix", "[basic]") {
    setvbuf(stdout, NULL, _IONBF, 0);
    bool res = false;

    SECTION("infix to postfix 1: OP_LT, OP_EQ, OP_AND, OP_OR") {
        // "age < 29 and name = Jane or name = Alice";
        where_args_cond_t test_data[] = {
            {"age", OP_LT, {.s="", .i=29, .f=0.0}},
            {"", OP_AND, {.s="", .i=0, .f=0.0}},
            {"name", OP_EQ, {.s="Jane", .i=0, .f=0.0}},
            {"", OP_OR, {.s="", .i=0, .f=0.0}},
            {"name", OP_EQ, {.s="Alice", .i=0, .f=0.0}},
        };

        where_args_cond_t ref_data[] = {
                {"age", OP_LT, {.s="", .i=29, .f=0.0}},
                {"name", OP_EQ, {.s="Jane", .i=0, .f=0.0}},
                {"", OP_AND, {.s="", .i=0, .f=0.0}},
                {"name", OP_EQ, {.s="Alice", .i=0, .f=0.0}},
                {"", OP_OR, {.s="", .i=0, .f=0.0}},
        };
        const std::size_t len = sizeof(test_data) / sizeof(where_args_cond_t);
        auto *res_data = (where_args_cond_t*)calloc(len, sizeof(where_args_cond_t));

        rpn_infix_to_postfix(test_data, res_data, len);
        res = compare_batch_where_args_cond_t(res_data, ref_data, len);
        REQUIRE(res);

        free(res_data);
    }
    SECTION("infix to postfix 2: with parenthesis") {
        // "age < 29 and ( name = Jane or name = Alice )";
        where_args_cond_t test_data[] = {
                {"age", OP_LT, {.s="", .i=29, .f=0.0}},
                {"", OP_AND, {.s="", .i=0, .f=0.0}},
                {"", OP_OPEN_PARENTHESIS, {.s="", .i=0, .f=0.0}},
                {"name", OP_EQ, {.s="Jane", .i=0, .f=0.0}},
                {"", OP_OR, {.s="", .i=0, .f=0.0}},
                {"name", OP_EQ, {.s="Alice", .i=0, .f=0.0}},
                {"", OP_CLOSE_PARENTHESIS, {.s="", .i=0, .f=0.0}},
        };

        where_args_cond_t ref_data[] = {
                {"age", OP_LT, {.s="", .i=29, .f=0.0}},
                {"name", OP_EQ, {.s="Jane", .i=0, .f=0.0}},
                {"name", OP_EQ, {.s="Alice", .i=0, .f=0.0}},
                {"", OP_OR, {.s="", .i=0, .f=0.0}},
                {"", OP_AND, {.s="", .i=0, .f=0.0}},
        };
        const std::size_t test_len = sizeof(test_data) / sizeof(where_args_cond_t);
        const std::size_t ref_len = sizeof(ref_data) / sizeof(where_args_cond_t);
        auto *res_data = (where_args_cond_t*)calloc(ref_len, sizeof(where_args_cond_t));

        rpn_infix_to_postfix(test_data, res_data, test_len);
        res = compare_batch_where_args_cond_t(res_data, ref_data, ref_len);
        REQUIRE(res);

        free(res_data);
    }
}
TEST_CASE("RPN evaluation", "[basic]") {
    setvbuf(stdout, NULL, _IONBF, 0);

    SECTION("Evaluation") {
        table_col_t cols[] = {
        {.name="name", .type=TABLE_STRING, .enable=true},
        {.name="age", .type=TABLE_INT, .enable=true},
        {.name="height", .type=TABLE_FLOAT, .enable=false}
        };

        table_data_t t = {
            .cols=cols,
            .rows=nullptr,
            .col_len=3,
            .col_enable_cnt=2,
            .row_len=5
        };
        char test_data[5][3][10] = {
            {"John", "30", "170"},
            {"Jane", "25", "165"},
            {"Alice", "28", "180"},
            {"Bob", "31", "173"},
            {"Charlie", "29", "160"}
        };

        where_args_cond_t postfix_conditions[] = {
            {"age", OP_LT, {.s="", .i=29, .f=0.0}},
            {"name", OP_EQ, {.s="Jane", .i=0, .f=0.0}},
            {"name", OP_EQ, {.s="Alice", .i=0, .f=0.0}},
            {"", OP_OR, {.s="", .i=0, .f=0.0}},
            {"", OP_AND, {.s="", .i=0, .f=0.0}},
        };
        size_t post_cond_len = sizeof(postfix_conditions) / sizeof(where_args_cond_t);

        std::vector<bool> res;
        std::vector<bool> res_ref = {false, true, true, false, false};

        bool ans;
        for (int i=0; i<5; i++) {
            char *cells[3];
            for (int j = 0; j < 3; ++j) {
                cells[j] = test_data[i][j];
            }

            ans = rpn_evaluate_where_conditions(&t, cells, postfix_conditions, post_cond_len);
            res.push_back(ans);
        }
        REQUIRE(res == res_ref);
    }
}
