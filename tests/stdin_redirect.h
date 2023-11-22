//
// Created by MCS51_M2 on 2023/11/22.
//

#ifndef SIMPLE_DATABASE_STDIN_REDIRECT_H
#define SIMPLE_DATABASE_STDIN_REDIRECT_H

#include <cstring>
#include <cstdio>

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

std::string stdin_write_data(StdinRedirector& sr, prompt_buf_t *prompt_buf, std::string s) {
    // stdin write data
    std::string query = s;
    sr.write(s);

    // process data
    read_input(prompt_buf);
    query.pop_back();       // Delete '\n'
    return query;
}

#endif //SIMPLE_DATABASE_STDIN_REDIRECT_H
