#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define PAGES 2
#define ROWSIZE (getpagesize() * PAGES)
#define RANDMAX ROWSIZE
#define TRIES 10
#define FNAME "mmap.dat"

int nb_access = 0;

void handler(int signal, siginfo_t* siginfo, void* uap) {
  nb_access++;
  if (mprotect(siginfo->si_addr - ((int)siginfo->si_addr % getpagesize()), getpagesize(), PROT_READ) == -1) {
    fprintf(stderr, "error calling mprotect()! %s\n", strerror(errno));
    exit(1);
  }
  printf("@%p\n", siginfo->si_addr);
}


int main() { 
  srand(time(NULL));

  int f = open(FNAME, O_RDWR|O_TRUNC|O_CREAT, S_IRUSR|S_IWUSR);
  if (f == -1) {
    fprintf(stderr, "error calling open()! %s\n", strerror(errno));
    exit(1);
  }

  /* int *matrix;                                                                                */
  /* if (posix_memalign((void**)&matrix, getpagesize(), ROWSIZE * ROWSIZE * sizeof(int)) != 0) { */
  /*   fprintf(stderr, "error calling posix_memalign()! %s\n", strerror(errno));                 */
  /* }                                                                                           */
  int *matrix = malloc(ROWSIZE * ROWSIZE * sizeof(int));
  for (int i = 0; i < ROWSIZE * ROWSIZE; i++)
    matrix[i] = rand() % RANDMAX;

  if (write(f, matrix, ROWSIZE * ROWSIZE * sizeof(int)) == -1) {
    fprintf(stderr, "error calling write()! %s\n", strerror(errno));
    exit(1);
  }

  int mfile_size = ROWSIZE * ROWSIZE * sizeof(int);
  int *mfile = (int *)mmap(0, mfile_size, PROT_READ, MAP_SHARED, f, 0);
  if (mfile == MAP_FAILED) {
    fprintf(stderr, "error calling mmap()! %s\n", strerror(errno));
    exit(1);
  }

  if (mprotect(mfile, mfile_size, PROT_NONE) == -1) {
    fprintf(stderr, "error calling mprotect()! %s\n", strerror(errno));
    exit(1);
  }

  struct sigaction act;
  memset(&act, 0, sizeof(struct sigaction));
  sigemptyset(&act.sa_mask);
  act.sa_sigaction = handler;
  act.sa_flags = SA_SIGINFO;
  if (sigaction(SIGSEGV, &act, NULL) < 0 || sigaction(SIGBUS, &act, NULL) < 0) {
    fprintf(stderr, "error calling sigaction()! %s\n", strerror(errno));
    exit(1);
  }

  for (int i = 0; i < TRIES; i++) {
    int x = rand() % RANDMAX, y = rand() % RANDMAX;
    int *p = mfile + y * ROWSIZE + x;
    printf("@%p = %d\n", p, *p);
  }

  printf("%f%%\n", (nb_access / (float)PAGES) * 100);
  
  if (munmap(mfile, mfile_size) == -1) {
    fprintf(stderr, "error calling munmap()! %s\n", strerror(errno));
    exit(1);
  }

  if (close(f) == -1) {
    fprintf(stderr, "error calling close()! %s\n", strerror(errno));
    exit(1);
  }

  free(matrix);
  
  return 0;
}
