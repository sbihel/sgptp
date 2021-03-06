#include "userlib/syscall.h"
#include "userlib/libnachos.h"

#define BSIZE 10 /* buffer size */

#define PROD_ITEM 1
#define CONS_REPL 0

#define NB_PROD 3 /* number of producer threads */
#define NB_CONS 3 /* number of consumer threads */
                  /* needs NB_PROD=NB_CONS */

#define TLIM 40 /* loop count of producers/consumers */

char buf[BSIZE]; // buffer

SemId occupied; // to wait when the buffer is full
SemId empty;    // to wait when the buffer is empty

int nextin;  // buffer head
int nextout; // buffer tail

LockId pmut; // producers mutex
LockId cmut; // consumers mutex

int balance_cons;    // |NB_PROD-NB_CONS|
int nb_actions_cons; // number of produce/consume
int balance_prod;    // |NB_PROD-NB_CONS|
int nb_actions_prod; // number of produce/consume

void producer();
void consumer();

int main() {
  occupied = SemCreate("occupied", 0);
  empty    = SemCreate("empty", BSIZE);

  pmut     = LockCreate("pmut");
  cmut     = LockCreate("cmut");

  nextin  = 0;
  nextout = 0;

  balance_prod    = 0;
  balance_cons    = 0;
  nb_actions_prod = 0;
  nb_actions_cons = 0;

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

  n_printf(">>> balance: %d\n", balance_cons + balance_prod);
  n_printf(">>> num actions: %d\n", nb_actions_cons + nb_actions_prod);

  return 0;
}

void producer() {
  int i;
  for (i=0; i<TLIM; i++) {
    LockAcquire(pmut);

    P(empty);

    /* if (balance > BSIZE) {    */
    /*   n_printf("overflow\n"); */
    /* }                         */

    nb_actions_prod++;
    balance_prod++;

    V(occupied);
    LockRelease(pmut);
  }
}

void consumer() {
  int i;
  for (i=0; i<TLIM; i++) {
    LockAcquire(cmut);

    P(occupied);

    /* if (balance <= 0) {        */
    /*   n_printf("underflow\n"); */
    /* }                          */

    nb_actions_cons++;
    balance_cons--;

    V(empty);
    LockRelease(cmut);
  }
}
