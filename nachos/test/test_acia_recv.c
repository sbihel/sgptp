#include "userlib/syscall.h"
#include "userlib/libnachos.h"

int main() {
  char buf[256];
  TtyReceive(buf, 256);

  n_printf(buf);

  return 0;
}
