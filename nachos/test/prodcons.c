#include "userlib/syscall.h"
#include "userlib/libnachos.h"
#include <stdarg.h>

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
  for (i = 0; i < NB_THREAD; i++) {
    char str[20];
    n_snprintf(str, 20, "prod%d", i);
    prod[i] = threadCreate(str, producer, i, 10, i);
    n_snprintf(str, 20, "cons%d", i);
    cons[i] = threadCreate(str, consumer);
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

void producer(va_list arguments) {
  /*va_list ap2;                                             */
  /*va_copy(ap2, arguments);                                 */
  /*n_printf("||||||||||||||||||||||\n");                    */
  /*n_printf("%d||||||||||||||||||||||\n", va_arg(ap2, int));*/
  /*va_end(ap2);                                             */
  // Implementing a wraper might be hard
  int test  = va_arg(arguments, int);
  int test2 = va_arg(arguments, int);
  int test3 = va_arg(arguments, int);
  V(wait_broadcast);
  /*CondWait(start);*/

  test++;
  test2++;
  test3++;
  n_printf("test: %d\n", test);
  n_printf("test2: %d\n", test2);
  n_printf("test3: %d\n", test3);

  int i;
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
