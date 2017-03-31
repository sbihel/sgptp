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
#define LOOP 10
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

int* buffer_flag;

void consumer(int sem_id) {
  // Create a shared memory segment
  int shm_id = shmget(KEY_SEG, SIZE * sizeof(int), 0660|IPC_CREAT);
  if (shm_id == -1) {
    exit(1);
  } else {
    // pointer to shm
    int *shared_int = (int *)shmat(shm_id, 0, SHM_RDONLY);

    int i;
    buffer_flag = shared_int;
    do {
      if (semop(sem_id, &cons_P, 1) == -1)  {
        exit(1);
      }

      // start critical section
      if (semop(sem_id, &shm_P, 1) == -1)  {
        exit(1);
      }

      // read the current counter
      i = *((int *)(buffer_flag));
      printf("%d\n", i);
      fflush(stdout);

      // decrement flag
      buffer_flag = (int*)(buffer_flag + ((linked_list*)buffer_flag)->offset);

      // end critical section
      if (semop(sem_id, &shm_V, 1) == -1)  {
        exit(1);
      }

      // can produce
      if (semop(sem_id, &prod_V, 1) == -1)  {
        exit(1);
      }
    } while (i < LOOP);

    // detach shm
    if (shmdt(shared_int))  {
      exit(1);
    }

    // delete shm
    if (shmctl(shm_id, 0, 0))  {
      exit(1);
    }
  }

  // close semaphores
  if (semctl(sem_id, 0, IPC_RMID, 0))  {
    exit(1);
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
      consumer(sem_id);
    }
  }

  return 0;
}
