//
// Created by MCS51_M2 on 2023/11/22.
//

#ifndef SIMPLE_DATABASE_IO_REDIRECT_H
#define SIMPLE_DATABASE_IO_REDIRECT_H

#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <regex>


// Need to declare IORedirector variable inside Catch2 SECTION, otherwise it'll not show message correctly
class IORedirector {
private:
    int pipefd_stdin[2];
    int pipefd_stdout[2];
    int pipefd_stderr[2];
    int orig_stdin;
    int orig_stdout;
    int orig_stderr;
    FILE* input_stream;
    FILE* output_stream;
    FILE* error_stream;

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

        // Redirect stderr
        if (pipe(pipefd_stderr) == -1) {
            throw std::runtime_error("Failed to create pipe for stderr");
        }
        orig_stderr = dup(STDERR_FILENO);
        if (orig_stderr == -1 || dup2(pipefd_stderr[1], STDERR_FILENO) == -1) {
            throw std::runtime_error("Failed to duplicate stderr");
        }
        fcntl(pipefd_stderr[0], F_SETFL, O_NONBLOCK);
        error_stream = fdopen(pipefd_stderr[0], "r");
        if (!error_stream) {
            throw std::runtime_error("Failed to open pipe for reading from stderr");
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

        // Close and restore stderr
        fclose(error_stream);
        dup2(orig_stderr, STDERR_FILENO);
        close(orig_stderr);
        close(pipefd_stderr[0]);
        close(pipefd_stderr[1]);
    }

    void write_stdin(const std::string &data) {
        if (fputs(data.c_str(), input_stream) == EOF) {
            throw std::runtime_error("Failed to write data to pipe");
        }
        fflush(input_stream);
    }

    std::string read_stdout() {
        std::string output;
        const int bufferSize = 256;
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

    std::string read_stderr() {
        std::string output;
        const int bufferSize = 256;
        char buffer[bufferSize];

        ssize_t bytesRead = 0;
        do {
            bytesRead = read(pipefd_stderr[0], buffer, bufferSize - 1);
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0'; // Null-terminate the string
                output += buffer;
            }
        } while (bytesRead > 0);

        return output;
    }

    void flush() {
        read_stdout();
        read_stderr();
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

std::string filter_out_catch2_string(const std::string& s) {
    // Main catch2 frame
    std::regex pattern1("\\n\\s*<Section name.*?>\\n");
    std::string s1 = std::regex_replace(s, pattern1, "");

    // Leading and trailing `"`
    std::regex pattern2("^\"|\"$");
    std::string s2 = std::regex_replace(s1, pattern1, "");
    return s2;
}

bool compare_io_response_str(std::string src_str, const std::string& ref_str) {
    src_str = filter_out_catch2_string(src_str);
    return (src_str == ref_str);
}

#endif //SIMPLE_DATABASE_IO_REDIRECT_H
