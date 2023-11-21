#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/utsname.h>

#include "main.h"
#include "prompt.h"
#include "inputs.h"

void check_environment_info(void) {
    fprintf(stdout, "Current Environment Info: \n");
    struct utsname buffer;

    errno = 0;
    if (uname(&buffer) < 0) {
        perror("uname");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "system name = %s\n", buffer.sysname);
    fprintf(stdout, "node name   = %s\n", buffer.nodename);
    fprintf(stdout, "release     = %s\n", buffer.release);
    fprintf(stdout, "version     = %s\n", buffer.version);
    fprintf(stdout, "machine     = %s\n", buffer.machine);
}

int main() {
//    check_environment_info();
    query_state_t query_state = {.state=INIT};

    prompt_buf_t *prompt_buf = new_prompt_buf();
    while (1) {
        print_prompt();
        read_input(prompt_buf);

        check_commands(prompt_buf, &query_state);

        if (query_state.state == EXIT) {
            fprintf(stdout, "Bye Bye \n");
            break;
        }
    }

    free_prompt_buf(prompt_buf);
    printf("Yooo \n");
    return EXIT_SUCCESS;
}
