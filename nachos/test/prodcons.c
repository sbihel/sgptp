#include "userlib/syscall.h"
#include "userlib/libnachos.h"

#define BSIZE 4
#define THNUM 1

char buf[BSIZE];
SemId occupied;
SemId empty;
int nextin;
int nextout;
LockId pmut;
LockId cmut;

void producer();
void consumer();

int main() {
  occupied = SemCreate("occupied", 0);
  empty = SemCreate("empty", BSIZE);
  pmut = LockCreate("pmut");
  cmut = LockCreate("cmut");
  nextin = nextout = 0;

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
  
  return 0;
}


void producer() {
  char item = 1;

  while(1) {    
    P(empty);
  
    LockAcquire(pmut);
    
    buf[nextin] = item;
    nextin++;
    nextin %= BSIZE;
  
    LockRelease(pmut);
    P(occupied);
  }
}

void consumer() {
  char item;

  while(1) {
    P(occupied);
    
    LockAcquire(cmut);

    item = buf[nextout];
    nextout++;
    nextout %= BSIZE;

    LockRelease(cmut);
    
    V(empty);
  }
}
