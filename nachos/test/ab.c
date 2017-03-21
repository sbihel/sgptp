#include "userlib/syscall.h"
#include "userlib/libnachos.h"

LockId lock;

int val;

void a();
void b();

int main() {
  lock = LockCreate("lock");

  val = 0;
  
  ThreadId a_t = threadCreate((char *)"A", a);
  ThreadId b_t = threadCreate((char *)"B", b);

  Join(a_t);
  Join(b_t);

  LockDestroy(lock);

  return 0;
}

void a() {
  while (1) {
    LockAcquire(lock);
    if (val != 'a') {
      n_printf("switch from '%c' to 'a'\n", val);
      val = 'a';
    } else {
      Yield();
    }
    LockRelease(lock);
  }
}

void b() {
  while (1) {
    LockAcquire(lock);
    if (val != 'b') {
      n_printf("switch from '%c' to 'b'\n", val);
      val = 'b';
    } else {
      Yield();
    }
    LockRelease(lock);
  }
}
