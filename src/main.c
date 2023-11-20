#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/utsname.h>

#include "main.h"
#include "prompt.h"
#include "inputs.h"

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

int main() {
//    check_environment_info();

    prompt_buf_t *prompt_buf = new_prompt_buf();
    while (1) {
        print_prompt();
        read_input(prompt_buf);

        check_command(prompt_buf);
    }

    return EXIT_SUCCESS;
}
