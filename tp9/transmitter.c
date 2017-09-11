#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define QUEUE_ID 1337
#define MAX_MSG_SIZE 42
#define NUM_MESSAGES 13

struct msgbuf {
  long mtype;
  char mtext[1];
};

int main() {
  int queue_id = msgget(QUEUE_ID, IPC_CREAT | IPC_EXCL | 0600);
  if (queue_id == -1) {
    perror("main: msgget");
    exit(1);
  }
  printf("message queue created, queue id '%d'.\n", queue_id);

  struct msgbuf* msg = (struct msgbuf*)malloc(sizeof(msg)+1+MAX_MSG_SIZE);

  for (int i=0; i < NUM_MESSAGES; i++) {
    msg->mtype = 1;
    sprintf(msg->mtext, "hello world - %d", i);
    int rc = msgsnd(queue_id, msg, strlen(msg->mtext)+1, 0);
    if (rc == -1) {
      perror("main: msgsnd");
      exit(1);
    }
  }

  printf("generated %d messages, exiting.\n", NUM_MESSAGES);

  return 0;
}
