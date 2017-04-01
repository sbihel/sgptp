#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define KEY_SEG 35
#define KEY 1337
#define PROD_SEM 0
#define CONS_SEM 1
#define SHM_SEM 2
#define LOOP 15
#define SIZE 5

struct sembuf prod_P = {PROD_SEM, -1, 0};
struct sembuf prod_V = {PROD_SEM, 1, 0};
struct sembuf cons_P = {CONS_SEM, -1, 0};
struct sembuf cons_V = {CONS_SEM, 1, 0};
struct sembuf shm_P = {SHM_SEM, -1, 0};
struct sembuf shm_V = {SHM_SEM, 1, 0};

typedef struct linked_list {
  int value;
  int offset;
} linked_list;

linked_list *buffer_flag;

void producer(int sem_id) {
  // Create a shared memory segment
  int shm_id = shmget(KEY_SEG, SIZE * sizeof(linked_list), 0660 |
      IPC_CREAT);
  if (shm_id == -1) {
    perror("shmget");
    exit(1);
  }
  // pointer to shm
  int *shared_int = (int *)shmat(shm_id, 0, SHM_RND);

  // init 'linked list'
  {
  linked_list l = {.value = 0, .offset = 0};
  *((linked_list*)(shared_int + (SIZE-1)*sizeof(linked_list))) = l;
  }
  for (int k = 0; k < SIZE-1; k++) {
    linked_list l2 = {.value = 0, .offset = sizeof(linked_list)};
    *((linked_list*)(shared_int + k*sizeof(linked_list))) = l2;
  }

  int i = 0;
  buffer_flag = (linked_list*)shared_int;
  while (i < LOOP) {
    printf("%d\n", i);
    fflush(stdout);

    if (semop(sem_id, &prod_P, 1) == -1)
      exit(1);

    // start critical section
    if (semop(sem_id, &shm_P, 1) == -1)
      exit(1);

    // write current counter
    buffer_flag->value = i;

    // moving buffer_flag to offset
    if (!buffer_flag->offset)
      buffer_flag = (linked_list*)shared_int;
    else
      buffer_flag = buffer_flag + buffer_flag->offset;

    if (semop(sem_id, &shm_V, 1) == -1)
      exit(1);
    // end critical section

    // can consume
    if (semop(sem_id, &cons_V, 1) == -1)
      exit(1);

    i++;
  }
}

int main() {
  int sem_id = semget(KEY, 3, 0660 | IPC_CREAT);
  if (sem_id == -1) {
    perror("sem_id");
    exit(1);
  }

  // initialise sem
  if ((semctl(sem_id, PROD_SEM, SETVAL, 1) == -1) ||
      (semctl(sem_id, CONS_SEM, SETVAL, 0) == -1) ||
      (semctl(sem_id, SHM_SEM, SETVAL, 1) == -1))
    exit(1);

  producer(sem_id);

  return 0;
}
