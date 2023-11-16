#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/utsname.h>

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
    check_environment_info();

    return EXIT_SUCCESS;
}
