#include "userlib/syscall.h"
#include "userlib/libnachos.h"

#define BSIZE 1
#define THNUM 100

char buf[BSIZE];
SemId occupied;
SemId empty;
int nextin;
int nextout;
LockId pmut;
LockId cmut;

int balance;

void producer();
void consumer();

int main() {
  occupied = SemCreate("occupied", 0);
  empty = SemCreate("empty", BSIZE);
  pmut = LockCreate("pmut");
  cmut = LockCreate("cmut");
  nextin = nextout = 0;

  balance = 0;

  ThreadId prod[THNUM], cons[THNUM];

  int i;
  for (i = 0; i < THNUM; i++) {
    prod[i] = threadCreate("prod", producer);
    cons[i] = threadCreate("cons", consumer);
  }
  for (i = 0; i < THNUM; i++) {
    Join(prod[i]);
    Join(cons[i]);
  }

  SemDestroy(occupied);
  SemDestroy(empty);
  LockDestroy(pmut);
  LockDestroy(cmut);
  
  n_printf("> %d\n", balance);
  
  return 0;
}


void producer() {
  char item = 1;
  
  int i;
  for (i=0; i<100; i++) {    
    P(empty);
  
    LockAcquire(pmut);
    
    buf[nextin] = item;
    nextin++;
    nextin %= BSIZE;

    balance++;
    n_printf("%d\n", balance);

    LockRelease(pmut);
    P(occupied);
  }

  Exit(0);
}

void consumer() {
  char item;

  int i;
  for (i=0; i<100; i++) {
    P(occupied);
    
    LockAcquire(cmut);

    item = buf[nextout];
    nextout++;
    nextout %= BSIZE;

    balance--;
    n_printf("%d\n", balance);

    LockRelease(cmut);
    
    V(empty);
  }

  Exit(0);
}
