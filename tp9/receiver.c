#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define QUEUE_ID 1337
#define MAX_MSG_SIZE 42
#define NUM_MESSAGES 13
#define MTYPE =

struct msgbuf {
  long mtype;
  char mtext[1];
};

int main() {
  int queue_id = msgget(QUEUE_ID, 0);
  if (queue_id == -1) {
    perror("main: msgget");
    exit(1);
  }
  printf("message queue opened, queue id '%d'.\n", queue_id);

  struct msgbuf* msg = (struct msgbuf*)malloc(sizeof(struct msgbuf)+MAX_MSG_SIZE);

  for (int i=1; i <= NUM_MESSAGES; i++) {
    int rc = msgrcv(queue_id, msg, MAX_MSG_SIZE+1, 0, 0);
    if (rc == -1) {
      perror("main: msgrcv");
      exit(1);
    }
    printf("read message: '%s'\n", msg->mtext);
    sleep(1);
  }

  msgctl(queue_id, IPC_RMID, 0);
}
