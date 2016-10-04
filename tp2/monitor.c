#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/types.h>

static int *glob_var;

int main() {
  pid_t pid;
  glob_var = mmap(NULL, sizeof *glob_var, PROT_READ | PROT_WRITE,
      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  *glob_var = 1;
  if ((pid = fork()) < 0) {
    perror("fork failure");
    exit(1);
  } else if (pid == 0) {
    ptrace(PT_TRACE_ME, 0, 0, 0);
    for(*glob_var = 0; *glob_var < 1337; (*glob_var)++) {
      usleep(10000);
    }
  } else {
    /*if(ptrace(PT_ATTACH, pid, 0, 0)) {*/
    if(ptrace(PTRACE_ATTACH, pid, 0, 0)) {
      perror("PTRACE_ATTACH");
      exit(1);
    }
    int buf;
    while(1) {
      /*printf("%d\n", *glob_var);*/
      /*if((buf = ptrace(PT_READ_D, pid, (void *)&glob_var, 0)) == -1) {*/
      if((buf = ptrace(PTRACE_PEEKDATA, pid, (void *)&glob_var, 0)) == -1) {
        perror("PTRACE_PEEKDATA");
        /*if(ptrace(PT_DETACH, pid, 0, 0))*/
        if(ptrace(PTRACE_DETACH, pid, 0, 0))
          perror("PTRACE_DETACH");
        exit(1);
      }
      printf("%d\n", buf);
    }
    /*if(ptrace(PT_DETACH, pid, 0, 0)) {*/
    if(ptrace(PTRACE_DETACH, pid, 0, 0)) {
      perror("PTRACE_DETACH");
      exit(1);
    }
    munmap(glob_var, sizeof *glob_var);
  }

  return 0;
}
