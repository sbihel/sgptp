#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
    pid_t pid;
    pid_t status;

    if ((pid = fork()) < 0) {
        perror("fork failure");
        exit(1);
    } else if (pid == 0) {
         execl("./partie2", "partie2", 0, (char *)0);
         perror("execl failure");
    } 
    
    wait(&status);
    printf("%d\n", WEXITSTATUS(status));

    exit(0);
}
