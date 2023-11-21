//
// Created by MCS51_M2 on 2023/11/19.
//

#include <catch2/catch_test_macros.hpp>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <memory>

#include "../src/inputs.h"
#include "../src/prompt.h"


class StdinRedirector {
private:
    int pipefd[2];
    int orig_stdin;
    FILE* input_stream;

public:
    StdinRedirector() {
        if (pipe(pipefd) == -1) {
            throw std::runtime_error("Failed to create pipe");
        }
        orig_stdin = dup(STDIN_FILENO);
        if (orig_stdin == -1 || dup2(pipefd[0], STDIN_FILENO) == -1) {
            throw std::runtime_error("Failed to duplicate stdin");
        }
        input_stream = fdopen(pipefd[1], "w");
        if (!input_stream) {
            throw std::runtime_error("Failed to open pipe for writing");
        }
    }

    ~StdinRedirector() {
        fclose(input_stream);
        dup2(orig_stdin, STDIN_FILENO);
        close(orig_stdin);
        close(pipefd[0]);
        close(pipefd[1]);
    }

    void write(const std::string &data) {
        if (fputs(data.c_str(), input_stream) == EOF) {
            throw std::runtime_error("Failed to write data to pipe");
        }
        fflush(input_stream);
    }
};

//std::string stdin_write_data(std::unique_ptr<StdinRedirector>& sr, prompt_buf_t *prompt_buf, std::string s) {
std::string stdin_write_data(StdinRedirector& sr, prompt_buf_t *prompt_buf, std::string s) {
    // stdin write data
    std::string query = s;
//    sr->write(s);
    sr.write(s);

    // process data
    read_input(prompt_buf);
    query.pop_back();       // Delete '\n'
    return query;
}

TEST_CASE("Check Commands", "[command]") {
//    std::unique_ptr<StdinRedirector> redirector = std::make_unique<StdinRedirector>();
    StdinRedirector redirector;

//    std::unique_ptr<MyClass> obj = std::make_unique<MyClass>();
    SECTION("Simple Command Test 1") {
//        StdinRedirector redirector;
//        std::unique_ptr<StdinRedirector> redirector = std::make_unique<StdinRedirector>();
        prompt_buf_t *prompt_buf = new_prompt_buf();

        std::string query = stdin_write_data(redirector, prompt_buf, "select * from db\n");
        REQUIRE(std::string(prompt_buf->buf) == query);
        REQUIRE(prompt_buf->len == query.length());

        free_prompt_buf(prompt_buf);
    }

    SECTION("Simple Command Test 2") {
//        StdinRedirector redirector;
//        std::unique_ptr<StdinRedirector> redirector = std::make_unique<StdinRedirector>();
        query_state_t query_state = {.state=INIT};
        prompt_buf_t *prompt_buf = new_prompt_buf();

        stdin_write_data(redirector, prompt_buf, "exit\n");
        check_commands(prompt_buf, &query_state);
        REQUIRE(query_state.state == EXIT);

        free_prompt_buf(prompt_buf);
    }
}


//        stdin_write_data(redirector, prompt_buf, "create");
//        check_commands(prompt_buf, &query_state);
//        REQUIRE(query_state.state == CREATE);
//
//        stdin_write_data(redirector, prompt_buf, "drop");
//        check_commands(prompt_buf, &query_state);
//        REQUIRE(query_state.state == DROP);
//
//        stdin_write_data(redirector, prompt_buf, "use");
//        check_commands(prompt_buf, &query_state);
//        REQUIRE(query_state.state == USE);
//
//        stdin_write_data(redirector, prompt_buf, "SELECT");
//        check_commands(prompt_buf, &query_state);
//        REQUIRE(query_state.state == SELECT);
