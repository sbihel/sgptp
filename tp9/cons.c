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

void consumer(int sem_id) {
  // Create a shared memory segment
  int shm_id = shmget(KEY_SEG, SIZE * sizeof(linked_list), 0660 |
      IPC_CREAT);
  if (shm_id == -1) {
    perror("shmget");
    exit(1);
  }
  // pointer to shm
  int *shared_int = (int *)shmat(shm_id, 0, SHM_RDONLY);

  int i = 0;
  buffer_flag = (linked_list*)shared_int;
  while (i < LOOP) {
    if (semop(sem_id, &cons_P, 1) == -1)
      exit(1);

    // start critical section
    if (semop(sem_id, &shm_P, 1) == -1)
      exit(1);

    // read the current counter
    printf("%d\n", buffer_flag->value);
    fflush(stdout);

    // move flag
    if (!buffer_flag->offset)
      buffer_flag = (linked_list*)shared_int;
    else
      buffer_flag = buffer_flag + (buffer_flag)->offset;

    // end critical section
    if (semop(sem_id, &shm_V, 1) == -1)
      exit(1);

    // can produce
    if (semop(sem_id, &prod_V, 1) == -1)
      exit(1);

    i++;
  }

  // detach shm
  if (shmdt(shared_int))
    exit(1);

  // delete shm
  if (shmctl(shm_id, 0, 0))
    exit(1);

  // close semaphores
  if (semctl(sem_id, 0, IPC_RMID, 0))
    exit(1);
}

int main() {
  // please launch producer before consumer
  int sem_id = semget(KEY, 3, 0660);
  if (sem_id == -1) {
    perror("sem_id");
    exit(1);
  }

  consumer(sem_id);

  return 0;
}
