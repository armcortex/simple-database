#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/utsname.h>

#include "main.h"
#include "prompt.h"

void check_environment_info(void) {
    printf("Current Environment Info: \n");
    struct utsname buffer;

    errno = 0;
    if (uname(&buffer) < 0) {
        perror("uname");
        exit(EXIT_FAILURE);
    }

    printf("system name = %s\n", buffer.sysname);
    printf("node name   = %s\n", buffer.nodename);
    printf("release     = %s\n", buffer.release);
    printf("version     = %s\n", buffer.version);
    printf("machine     = %s\n", buffer.machine);
}

void read_input(prompt_buf_t *prompt_buf) {
    ssize_t read_bytes = getline(&(prompt_buf->buf), &(prompt_buf->len), stdin);
    if (read_bytes <= 0) {
        printf("Reading Error \n");
        free_prompt_buf(prompt_buf);
        exit(EXIT_FAILURE);
    }

    // Delete '\n' newline
    prompt_buf->len = read_bytes - 1;
    prompt_buf->buf[read_bytes-1] = 0;
}

int main() {
//    check_environment_info();

    prompt_buf_t *prompt_buf = new_prompt_buf();
    while (1) {
        print_prompt();
        read_input(prompt_buf);

        printf("Read %zu bytes, Input: %s \n", prompt_buf->len, prompt_buf->buf);
    }

    return EXIT_SUCCESS;
}
