#define _GNU_SOURCE

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>


void signal_handler (int signo, siginfo_t *si, void *data) {
  switch (signo) {
  case SIGFPE:
    fprintf(stdout, "Caught SIGFPE\n");
    exit(0);
    break;
  default:
    fprintf(stdout, "default handler\n");
  }
}


int main() {
  struct sigaction sa, osa;

  sa.sa_sigaction = signal_handler;
  
  if (sigaction(SIGFPE, &sa, &osa) == -1) {
    perror("sigaction");
    exit(1);
  }
  
  float a = 1 / 0;
  printf("%f\n", a);
  
  return 0;
}
