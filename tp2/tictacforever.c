#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int state = 0;

void alarm_handler(int sig) {
  printf("%d\n", state);
  alarm(1);
}

int main() {
  signal(SIGALRM, alarm_handler);
  alarm(1);
  
  do {
    state++;
  } while(1);
  
  return 0;
}
