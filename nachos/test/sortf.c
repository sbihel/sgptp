#include "userlib/syscall.h"
#include "userlib/libnachos.h"

#define SIZE_BUFF 5

void sort(int *p, int size) {
  int i, j;
  for (i = 0; i < size - 1; ++i) {
    for (j = 0; j < size - i - 1; ++j) {
      if (p[j] > p[j + 1]) {
        int temp;
        temp = p[j];
        p[j] = p[j + 1];
        p[j + 1] = temp;
      }
    }
  }
}

int main(void) {
  OpenFileId f;
  char *buff;

  if ((f = Open("/numbers.dat")) == -1) {
    n_printf("could not open the file\n");
    Exit(1);
  }

  if ((buff = (char *)Mmap(f, SIZE_BUFF * sizeof(char))) == -1) {
    n_printf("could not map the file");
    Exit(1);
  }

  n_printf("file: %x, buffer: %d\n", f, buff);

  /* sort(buff, SIZE_BUFF); */

  int i;
  for (i = 0; i < SIZE_BUFF * 2; i++) {
    n_printf("%d: %c \n", &buff[i % SIZE_BUFF], buff[i % SIZE_BUFF]);
    buff[i] = ';';
  }

  Close(f);

  Exit(0);
  return 0;
}
