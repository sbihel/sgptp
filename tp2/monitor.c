#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <errno.h>

static int *glob_var;

int main() {
  pid_t pid;
  glob_var = mmap(NULL, sizeof *glob_var, PROT_READ | PROT_WRITE,
      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  *glob_var = 1;
  pid = fork();
  if (pid < 0) {
    perror("fork failure");
    exit(1);
  } else if (pid == 0) {  // child
    ptrace(PTRACE_TRACEME, 0, 0, 0);
    for(*glob_var = 0; *glob_var < 1337; (*glob_var)++) {
      usleep(5000);
    }
  } else {  // parent

    if(ptrace(PTRACE_ATTACH, pid, 0, 0) == -1) {
      perror("PTRACE_ATTACH");
      exit(1);
    }
    int wstatus;
    wait(&wstatus);  // should be WIFSTOPPED

    if(ptrace(PTRACE_CONT, pid, 0, SIGCONT) == -1) {
      perror("PTRACE_CONT");
      exit(1);
    }

    int buf;
    while(1) {
      kill(pid, SIGSTOP);
      wait(&wstatus);
      buf = ptrace(PTRACE_PEEKDATA, pid, glob_var, 0);
      if(buf == -1 && errno) {  // will be triggered when the child finishes
        perror("PTRACE_PEEKDATA");
        if(ptrace(PTRACE_DETACH, pid, 0, 0))
          perror("PTRACE_DETACH");
        exit(1);
      }
      if(ptrace(PTRACE_CONT, pid, 0, SIGCONT) == -1) {
        perror("PTRACE_CONT");
        exit(1);
      }
      printf("%d\n", buf);
      fflush(stdout);
      usleep(2500);
    }
    munmap(glob_var, sizeof *glob_var);
  }

  return 0;
}
