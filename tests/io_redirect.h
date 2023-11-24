//
// Created by MCS51_M2 on 2023/11/22.
//

#ifndef SIMPLE_DATABASE_IO_REDIRECT_H
#define SIMPLE_DATABASE_IO_REDIRECT_H

#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>


class IORedirector {
private:
    int pipefd_stdin[2];
    int pipefd_stdout[2];
    int orig_stdin;
    int orig_stdout;
    FILE* input_stream;
    FILE* output_stream;

public:
    IORedirector() {
        // Redirect stdin
        if (pipe(pipefd_stdin) == -1) {
            throw std::runtime_error("Failed to create pipe");
        }
        orig_stdin = dup(STDIN_FILENO);
        if (orig_stdin == -1 || dup2(pipefd_stdin[0], STDIN_FILENO) == -1) {
            throw std::runtime_error("Failed to duplicate stdin");
        }
        input_stream = fdopen(pipefd_stdin[1], "w");
        if (!input_stream) {
            throw std::runtime_error("Failed to open pipe for writing");
        }

        // Redirect stdout
        if (pipe(pipefd_stdout) == -1) {
            throw std::runtime_error("Failed to create pipe for stdout");
        }
        orig_stdout = dup(STDOUT_FILENO);
        if (orig_stdout == -1 || dup2(pipefd_stdout[1], STDOUT_FILENO) == -1) {
            throw std::runtime_error("Failed to duplicate stdout");
        }
        fcntl(pipefd_stdout[0], F_SETFL, O_NONBLOCK);
        output_stream = fdopen(pipefd_stdout[0], "r");
        if (!output_stream) {
            throw std::runtime_error("Failed to open pipe for reading from stdout");
        }
    }

    ~IORedirector() {
        // Close and restore stdin
        fclose(input_stream);
        dup2(orig_stdin, STDIN_FILENO);
        close(orig_stdin);
        close(pipefd_stdin[0]);
        close(pipefd_stdin[1]);

        // Close and restore stdout
        fclose(output_stream);
        dup2(orig_stdout, STDOUT_FILENO);
        close(orig_stdout);
        close(pipefd_stdout[0]);
        close(pipefd_stdout[1]);
    }

    void write_stdin(const std::string &data) {
        if (fputs(data.c_str(), input_stream) == EOF) {
            throw std::runtime_error("Failed to write data to pipe");
        }
        fflush(input_stream);
    }

    std::string read_stdout() {
        std::string output;
        const int bufferSize = 10;
        char buffer[bufferSize];

        ssize_t bytesRead = 0;
        do {
            bytesRead = read(pipefd_stdout[0], buffer, bufferSize - 1);
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0'; // Null-terminate the string
                output += buffer;
            }
        } while (bytesRead > 0);

        return output;
    }
};

std::string stdin_write_data(IORedirector& sr, prompt_buf_t *prompt_buf, std::string s) {
    // stdin write data
    std::string query = s;
    sr.write_stdin(s);

    // process data
    read_input(prompt_buf);
    query.pop_back();       // Delete '\n'
    return query;
}

#endif //SIMPLE_DATABASE_IO_REDIRECT_H
