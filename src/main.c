#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/utsname.h>

#include "main.h"
#include "prompt.h"
#include "inputs.h"
#include "helper_functions.h"
#include "db_config.h"
#include "test_fn_env.h"

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
    fprintf(stdout, "==============================\n\n");
}

int main_app() {
    setvbuf(stdout, NULL, _IONBF, 0);
    if (VERBOSE) {
        check_environment_info();
        check_current_path();
    }

    create_folder(WORKSPACE_PATH_FULL);

    query_state_t *query_state = query_state_construct();
    query_state->init(query_state);

    prompt_buf_t *prompt_buf = new_prompt_buf();
    while (1) {
        print_prompt();
        read_input(prompt_buf);

        check_commands(prompt_buf, query_state);
        if (VERBOSE) {
            fprintf(stdout, "Current query state: %s\n",
                    query_state_to_string(query_state->state));
        }

        if (query_state->state == EXIT) {
            fprintf(stdout, "Bye Bye \n");
            break;
        }
    }

    free_prompt_buf(prompt_buf);
    query_state->close(query_state);
    return EXIT_SUCCESS;
}

int main() {
    setvbuf(stdout, 0, _IONBF, 0);
    main_app();

    // test_canbus_convert();

    return 0;
}
