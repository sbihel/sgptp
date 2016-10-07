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

int main() {
  pid_t pid;
  int glob_var = 1;

  pid = fork();
  if (pid < 0) {
    perror("fork failure");
    exit(1);
  } else if (pid == 0) {  // child
    ptrace(PTRACE_TRACEME, 0, 0, 0);
    for(glob_var = 0; glob_var < 1337; glob_var++) {
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
      if(kill(pid, SIGSTOP) == -1) {
        printf("Child has died.\n");
        break;
      }
      // even if the child has been killed there is a STOP signal emited except
      // for SIGKILL, so we avoid blocking on wait()
      // But apparently kill() never returns -1 even after sending SIGKILL
      wait(&wstatus);

      buf = ptrace(PTRACE_PEEKDATA, pid, &glob_var, 0);
      if(buf == -1 && errno) {
        if(errno == ESRCH) {  // We're sure it means child's death as it is stopped
          printf("Child has finished/died.\n");
          break;
        } else {
          perror("PTRACE_PEEKDATA");
          if(ptrace(PTRACE_DETACH, pid, 0, 0))
            perror("PTRACE_DETACH");
          exit(1);
        }
      }
      if(ptrace(PTRACE_CONT, pid, 0, SIGCONT) == -1) {
        perror("PTRACE_CONT");
        exit(1);
      }
      printf("%d\n", buf);
      fflush(stdout);
      usleep(2500);
    }
  }

  return 0;
}
