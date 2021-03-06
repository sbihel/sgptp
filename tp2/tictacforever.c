#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <setjmp.h>

int state = 0;
jmp_buf env;

sigset_t x;

void alarm_handler(int sig) {
  printf("%d\n", state);
  alarm(1);
}

void interupt_handler(int sig) {
  state = 0;
  sigprocmask(SIG_UNBLOCK, &x, NULL);
  longjmp(env, 12345789);
  // appenrently there's no need to unblock/unmask the signal (on os x)
}

int main() {
  sigemptyset(&x);
  sigaddset(&x, SIGINT);

  signal(SIGALRM, alarm_handler);
  signal(SIGINT, interupt_handler);
  alarm(1);

  setjmp(env);

  do {
    state++;
    usleep(10000);
  } while(1);

  return 0;
}
