#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define KEY_SEG 35
#define PROD_SEM 0
#define CONS_SEM 1
#define SHM_SEM 2
#define LOOP 5
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

int *buffer_flag;

void producer(int sem_id) {
  int shm_id = shmget(KEY_SEG, SIZE * sizeof(int), 0660 | IPC_CREAT);
  if (shm_id == -1) {
    exit(1);
  } else {
    int *shared_int = (int *)shmat(shm_id, 0, SHM_RND);

    linked_list l = {.value = 0, .offset = 0};
    *((linked_list*)(shared_int + (SIZE-1)*sizeof(linked_list))) = l;
    for (int k = 1; k < SIZE; k++) {
      linked_list l2 = {.value = 0, .offset = sizeof(linked_list)};
      *((linked_list*)(shared_int + k*sizeof(linked_list))) = l2;
    }

    int i = 0;
    buffer_flag = shared_int;
    while (i < LOOP) {
      printf("%d\n", i);
      fflush(stdout);

      if (semop(sem_id, &prod_P, 1) == -1) {
        exit(1);
      }

      // start critical section
      if (semop(sem_id, &shm_P, 1) == -1) {
        exit(1);
      }

      // write current counter
      *((int *)(buffer_flag)) = i;

      // moving buffer_flag to offset
      buffer_flag = (int*)(buffer_flag + ((linked_list*)buffer_flag)->offset);

      if (semop(sem_id, &shm_V, 1) == -1) {
        exit(1);
      }
      // end critical section

      // can consume
      if (semop(sem_id, &cons_V, 1) == -1) {
        exit(1);
      }

      i++;
    }

    // detach shm
    if (shmdt(shared_int)) {
      exit(1);
    }
  }
}

int main() {
  int sem_id = semget(IPC_PRIVATE, 3, 0660);
  if (sem_id == -1) {
    exit(1);
  } else {
    // initialise sem
    if ((semctl(sem_id, PROD_SEM, SETVAL, 1) == -1) |
        (semctl(sem_id, CONS_SEM, SETVAL, 0) == -1) |
        (semctl(sem_id, SHM_SEM, SETVAL, 1) == -1)) {
      exit(1);
    } else {
        producer(sem_id);
    }
  }

  return 0;
}
