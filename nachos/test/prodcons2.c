#include "userlib/syscall.h"
#include "userlib/libnachos.h"

#define MAX 10
char queue[MAX];
int head = 0, tail = 0;
SemId nchars, nholes;
LockId mutex;

void producer(void) {
  while (1) {
    P(nholes);
    LockAcquire(mutex);

    if (queue[head] == 'P') {
      n_printf("unconsumed!\n");
    }
    n_printf("produce\n");
    queue[head] = 'P';
    head = (head + 1) % MAX;

    LockRelease(mutex);
    V(nchars);
  }
}

void consumer(void) {
  while (1) {
    P(nchars);
    LockAcquire(mutex);

    if (queue[tail] == 'C') {
      n_printf("unproduced!\n");
    }
    n_printf("consume\n");
    queue[tail] = 'C';
    tail = (tail + 1) % MAX;

    LockRelease(mutex);
    V(nholes);
  }
}

int main() {
  nchars = SemCreate("nchars", 0);
  nholes = SemCreate("nholes", MAX);

  mutex = LockCreate("mutex");

  ThreadId consumers[10], producers[10];

  int i;
  char str[20];
  for (i = 0; i < 10; i++) {
    n_snprintf(str, 20, "prod%d", i);
    producers[i] = threadCreate(str, producer);
    n_snprintf(str, 20, "cons%d", i);
    consumers[i] = threadCreate(str, consumer);
  }

  for (i = 0; i < 10; i++) {
    Join(producers[i]);
    Join(consumers[i]);
  }

  SemDestroy(nchars);
  SemDestroy(nholes);

  LockDestroy(mutex);

  return 0;
}
