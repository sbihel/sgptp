#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>

#define N 64
#define K 8 // num threads

float M1[N][N], M2[N][N], M[N][N];

void *dot8(void *arg);

int main() {
  srand(time(NULL));
  for (size_t i = 0; i < N; i++) {
    //M1[i][i] = 1;
    //M2[i][i] = 1;
    for (size_t j = 0; j < N; j++) {
      M1[i][j] = rand();
      M2[i][j] = rand();
    }
  }

  for (size_t i = 0; i < K; i++) {
    pthread_t tid;
    pthread_create(&tid, NULL, dot8, (void *)i);
    pthread_join(tid, NULL); // the parent waits all threads
  }

  for (size_t i = 0; i < N; i++) {
    for (size_t j = 0; j < N; j++) {
      printf("%.0f ", M[i][j]);
    }
    printf("\n");
  }
  
  return 0;
}

void* dot8(void *arg) {
  size_t row = (size_t)arg;

  while(row < N) { // ugly
    for (size_t col = 0; col < N; col++) {    
      float sum = 0;
      for (size_t i = 0; i < N; i++) {
	sum += M1[row][i] * M2[i][col];
      }
      M[row][col] = sum;
    }
    row += K;
  }
  
  pthread_exit(0);
}
