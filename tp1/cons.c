#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void) {
    int fd;
    char *fifo = "/tmp/fifo";

    if ((fd = open(fifo, S_IFIFO | O_RDONLY)) == -1) {
        perror("open");
        exit(1);
    }

    int max_buf = 1024;
    char buf[max_buf];

    while (1) {
        switch (read(fd, buf, max_buf)) {
            case -1:
                perror("read");
                break;
            case 0:
                exit(0);
        }

        printf("> %s", buf);
    }

    close(fd);

    unlink(fifo);

    return 0;
}
