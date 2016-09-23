#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
    pid_t pid;

    int fd[2];
    if (pipe(fd)) {
        perror("pipe failure");
        exit(1);
    }

    int max_buf = 1024;
    char buf1[max_buf], buf2[max_buf];

    if ((pid = fork()) < 0) {
        perror("fork failure");
        exit(1);

    } else if (pid == 0) {
        while (fgets(buf1, max_buf, stdin) != 0 && buf1[0] != '\n') {
            close(fd[0]);
            write(fd[1], buf1, max_buf);
        }
        write(fd[1], "\0", sizeof("\0"));
        // '\0' is treated as null pointer in clang

    } else {
        while (1) {
            close(fd[1]);
            switch (read(fd[0], buf2, max_buf)) {
                case -1:
                    perror("read");
                    break;
                case 0:
                    exit(0);
            }
            printf("> %s", buf2);
        }
    }

    exit(0);
}
