//
// Created by MCS51_M2 on 2023/11/19.
//

#include <catch2/catch_test_macros.hpp>
#include <unistd.h>
#include <cstring>
#include <cstdio>

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

    void write(const std::string& data) {
        if (fputs(data.c_str(), input_stream) == EOF) {
            throw std::runtime_error("Failed to write data to pipe");
        }
        fflush(input_stream);
    }
};

TEST_CASE("Check Commands", "[command]") {
    StdinRedirector redirector;
    prompt_buf_t *prompt_buf = new_prompt_buf();

    SECTION("Simple Command Test: `select * from db`") {
        // Prepare testing data
        std::string query = "select * from db\n";
        redirector.write(query);

        // Make sure data source is correct
        read_input(prompt_buf);
        query.pop_back();       // Delete '\n'
        REQUIRE(std::string(prompt_buf->buf) == query);
        REQUIRE(prompt_buf->len == query.length());

        check_commands(prompt_buf);
    }

    free_prompt_buf(prompt_buf);
}
