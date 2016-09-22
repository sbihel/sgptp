#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void) {
    int fd;
    char * fifo = "/tmp/fifo";

    mknod(fifo, S_IFIFO | 0666, 0);

    fd = open(fifo, O_WRONLY);

    int max_buf = 1024;
    char buf[max_buf];
   
    while (fgets(buf, max_buf, stdin) != 0 && buf[0] != '\n') {
        write(fd, buf, max_buf);        
    }

    close(fd);

    return 0;
}
