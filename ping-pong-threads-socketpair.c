#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
  int sd;
  int i;
} thread_data_t;

void*
thread_routine(void* arg) {
  int sd = ((thread_data_t*) arg)->sd;
  int i = ((thread_data_t*) arg)->i;
  int value = 0;
  while (1) {
    if (read(sd, &value, sizeof(int)) == 0) {
      close(sd);
      return NULL;
    }
    if (i == 0) {
      value -= 3;
    } else {
      value += 5;
    }
    printf("%d\n", value);
    if (value <= 0 || value > 100) {
      close(sd);
      return NULL;
    }
    write(sd, &value, sizeof(int));
  }
}

int
main(int argc, char** argv) {
  int N = strtol(argv[1], NULL, 10);
  
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
  pthread_attr_setguardsize(&attr, 0);

  pthread_t threads[2];
  int sd[2];
  socketpair(AF_UNIX, SOCK_STREAM, 0, sd);
  thread_data_t data[2];
  data[0].i = 0;
  data[0].sd = sd[0];
  data[1].i = 1;
  data[1].sd = sd[1];
  for (size_t i = 0; i < 2; ++i) {
    pthread_create(&threads[i], &attr, thread_routine, &data[i]);
  }
  pthread_attr_destroy(&attr);

  write(sd[1], &N, sizeof(int));
  
  for (size_t i = 0; i < 2; ++i) {
    pthread_join(threads[i], NULL);
  }
  
  return 0;
}
