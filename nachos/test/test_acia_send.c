#include "userlib/syscall.h"
#include "userlib/libnachos.h"

int main() {
  char *buf = "ceci est un test";
  TtySend(buf);

  return 0;
}
