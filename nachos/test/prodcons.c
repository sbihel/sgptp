#include "userlib/syscall.h"
#include "userlib/libnachos.h"

#define BSIZE 1

#define PROD_ITEM 1
#define CONS_REPL 0

#define NB_THREAD 1

char buf[BSIZE];
SemId occupied;
SemId empty;
int nextin;
int nextout;
LockId pmut;
LockId cmut;

SemId lol;

CondId start;

int balance, nb_actions;

void producer();
void consumer();

int main() {
  occupied = SemCreate("occupied", 0);
  empty    = SemCreate("empty", BSIZE);
  pmut     = LockCreate("pmut");
  cmut     = LockCreate("cmut");
  nextin   = nextout = 0;
  
  start = CondCreate("starting gate");
  
  balance    = 0;
  nb_actions = 0;
  
  ThreadId prod[NB_THREAD], cons[NB_THREAD];
  
  int i;
  for (i = 0; i < NB_THREAD; i++) {
    char str[20];
    n_snprintf(str, 20, "prod%d", i);
    prod[i] = threadCreate(str, producer, i);
    n_snprintf(str, 20, "cons%d", i);
    cons[i] = threadCreate(str, consumer);
  }

  CondBroadcast(start);
  
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

void producer(int test) {
  test++;
  n_printf("lol, not sleeping\n");
  /*CondWait(start);*/
  n_printf("test: %d\n", test);

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
  /*CondWait(start);*/
  n_printf("lol, not sleeping\n");

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
