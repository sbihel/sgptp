#include "userlib/syscall.h"
#include "userlib/libnachos.h"

#define BSIZE 1

#define PROD_ITEM 1
#define CONS_REPL 0

#define NB_THREAD 2

char buf[BSIZE];
SemId occupied;
SemId empty;
int nextin;
int nextout;
LockId pmut;
LockId cmut;

SemId lol;

CondId start;
SemId  wait_broadcast;

int balance, nb_actions;

void producer();
void consumer();

int main() {
  occupied = SemCreate("occupied", 0);
  empty    = SemCreate("empty", BSIZE);
  pmut     = LockCreate("pmut");
  cmut     = LockCreate("cmut");
  nextin   = nextout = 0;

  start          = CondCreate("starting gate");
  wait_broadcast = SemCreate("wait_broadcast", 0);

  balance    = 0;
  nb_actions = 0;

  ThreadId prod[NB_THREAD], cons[NB_THREAD];

  int i;
  char *argv_test[3] = {"Hello", ", ", "world"};
  for (i = 0; i < NB_THREAD; i++) {
    char str[20];
    n_snprintf(str, 20, "prod%d", i);
    prod[i] = threadCreate(str, producer, 3, argv_test);
    n_snprintf(str, 20, "cons%d", i);
    cons[i] = threadCreate(str, consumer, 0, argv_test);
  }

  for(i = 0; i < 2 * NB_THREAD; i++) {
    P(wait_broadcast);
    // supposed to wait for all threads to wait on the condition but that's not
    // working (I know that this has edge cases)
  }
  CondBroadcast(start);
  SemDestroy(wait_broadcast);
  CondDestroy(start);

  for (i = 0; i < NB_THREAD; i++) {
    Join(prod[i]); // rendezvous: parent thread waits for children
    Join(cons[i]);
  }

  SemDestroy(occupied);
  SemDestroy(empty);
  LockDestroy(pmut);
  LockDestroy(cmut);

  n_printf(">>> balance: %d\n", balance);
  n_printf(">>> num actions: %d\n", nb_actions);

  return 0;
}

void producer(int argc, char *argv[]) {
  int i;
  n_printf("%d\n", argc);
  for(i = 0; i < argc; i++) {
    n_printf("%s", argv[i]);
  }
  n_printf("\n");

  V(wait_broadcast);
  /*CondWait(start);*/


  for (i=0; i<3; i++) {
    P(empty);

    LockAcquire(pmut);

    if (buf[nextin] == PROD_ITEM) {
      // producer overflow
      n_printf(">>> overflow\n");
    }
    buf[nextin] = PROD_ITEM; // produce item
    nextin++;
    nextin %= BSIZE;

    nb_actions++;
    balance++;
    n_printf(">>> producer: %d\n", balance);

    LockRelease(pmut);
    V(occupied);
  }
}

void consumer() {
  V(wait_broadcast);
  /*CondWait(start);*/

  int i;
  for (i=0; i<3; i++) {
    P(occupied);

    LockAcquire(cmut);

    if (buf[nextout] == CONS_REPL) {
      // consumer underflow
      n_printf(">>> undeflow\n");
    }

    buf[nextout] = CONS_REPL; // consume item

    nextout++;
    nextout %= BSIZE;

    nb_actions++;
    balance--;
    n_printf(">>> consumer: %d\n", balance);

    LockRelease(cmut);

    V(empty);
  }
}
