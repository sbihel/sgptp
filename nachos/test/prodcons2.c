#include "userlib/syscall.h"
#include "userlib/libnachos.h"

#define STACK_MAX 10

struct Stack {
  int data[STACK_MAX];
  int size;
};
typedef struct Stack Stack;

void Stack_Init(Stack *S) {
  S->size = 0;
}

void Stack_Push(Stack *S, int d) {
  if (S->size < STACK_MAX)
    S->data[S->size++] = d;
  else
    n_printf("Error: stack full\n");
}

void Stack_Pop(Stack *S) {
  if (S->size == 0)
    n_printf("Error: stack empty\n");
  else
    S->size--;
}

Stack *S;

SemId fillCount;
SemId emptyCount;

LockId buffer_mutex;

#define NUM_CONSUMERS 1
#define NUM_PRODUCERS 1

void producer();
void consumer();

int main() {
  fillCount = SemCreate("fillCount", 0);
  emptyCount = SemCreate("emptyCount", STACK_MAX);

  buffer_mutex = LockCreate("buffer_mutex");

  Stack_Init(S);

  ThreadId consumers[NUM_CONSUMERS], producers[NUM_PRODUCERS];

  int i;
  char str[20];
  for (i = 0; i < NUM_PRODUCERS; i++) {
    n_snprintf(str, 20, "prod%d", i);
    producers[i] = threadCreate(str, producer);
  }
  for (i = 0; i < NUM_CONSUMERS; i++) {
    n_snprintf(str, 20, "cons%d", i);
    consumers[i] = threadCreate(str, consumer);
  }

  for (i = 0; i < NUM_PRODUCERS; i++) {
    Join(producers[i]);
  }
  for (i = 0; i < NUM_CONSUMERS; i++) {
    Join(consumers[i]);
  }

  SemDestroy(fillCount);
  SemDestroy(emptyCount);

  LockDestroy(buffer_mutex);

  return 0;
}

void producer() {
  while (1) {
    P(emptyCount);
    LockAcquire(buffer_mutex);
    Stack_Push(S, 0);
    LockRelease(buffer_mutex);
    V(fillCount);
  }
}

void consumer() {
  while (1) {
    P(fillCount);
    LockAcquire(buffer_mutex);
    Stack_Pop(S);
    LockRelease(buffer_mutex);
    V(emptyCount);
  }
}
