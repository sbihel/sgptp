#include "userlib/syscall.h"
#include "userlib/libnachos.h"

#define BSIZE 10 /* buffer size */

#define PROD_ITEM 1
#define CONS_REPL 0

#define NB_PROD 3 /* number of producer threads */
#define NB_CONS 3 /* number of consumer threads */
                  /* needs NB_PROD=NB_CONS */

#define TLIM 10 /* loop count of producers/consumers */

char buf[BSIZE]; // buffer

SemId occupied; // to wait when the buffer is full
SemId empty;    // to wait when the buffer is empty

int nextin;  // buffer head
int nextout; // buffer tail

LockId pmut; // producers mutex
LockId cmut; // consumers mutex

int balance;    // |NB_PROD-NB_CONS|
int nb_actions; // number of produce/consume

void producer();
void consumer();

int main() {
  occupied = SemCreate("occupied", 0);
  empty    = SemCreate("empty", BSIZE);

  pmut     = LockCreate("pmut");
  cmut     = LockCreate("cmut");

  nextin  = 0;
  nextout = 0;

  balance    = 0;
  nb_actions = 0;

  ThreadId prod[NB_PROD], cons[NB_CONS];

  char str[20]; // thread name
  int i;
  // create threads
  for (i = 0; i < NB_PROD; i++) {
    n_snprintf(str, 20, "prod%d", i);
    prod[i] = threadCreate(str, producer);
  }
  for (i = 0; i < NB_CONS; i++) {
    n_snprintf(str, 20, "cons%d", i);
    cons[i] = threadCreate(str, consumer);
  } 

  // join threads
  for (i = 0; i < NB_PROD; i++) {
    Join(prod[i]);
  }
  for (i = 0; i < NB_CONS; i++) {
    Join(cons[i]);
  }

  // clean
  SemDestroy(occupied);
  SemDestroy(empty);
  LockDestroy(pmut);
  LockDestroy(cmut);

  n_printf(">>> balance: %d\n", balance);
  n_printf(">>> num actions: %d\n", nb_actions);

  return 0;
}

void producer() {  
  int i;
  for (i=0; i<TLIM; i++) {
    LockAcquire(pmut);

    P(empty);

    if (buf[nextin] == PROD_ITEM) {
      // producer overflow
      n_printf(">>> overflow\n");
    }

    buf[nextin] = PROD_ITEM; // produce item

    nextin++;
    nextin %= BSIZE;

    nb_actions++;
    balance++;

    V(occupied);
     LockRelease(pmut);
 }
}

void consumer() {  
  int i;
  for (i=0; i<TLIM; i++) {
     LockAcquire(cmut);
   P(occupied);

    if (buf[nextout] == CONS_REPL) {
      // consumer underflow
      n_printf(">>> undeflow\n");
    }

    buf[nextout] = CONS_REPL; // consume item

    nextout++;
    nextout %= BSIZE;

    nb_actions++;
    balance--;

    V(empty);
     LockRelease(cmut);
 }
}
