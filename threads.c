#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void*
thread_routine(void* arg) {
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  while (1) {}  //  loop: jump loop
  int num = (int64_t) arg;
  while (1) {
    printf("I'm thread #%d\n", num);
    //sleep(1);
  }
  return NULL;
}

int
main(int argc, char** argv) {
  int N = atoi(argv[1]);
  pthread_t threads[N];

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
  pthread_attr_setguardsize(&attr, 0);
  for (int i = 0; i < N; ++i) {
    pthread_create(&threads[i], &attr, thread_routine, (void*)(int64_t)i);
  }
  pthread_attr_destroy(&attr);

  sleep(10);
  for (int i = 0; i < N; ++i) {
    pthread_cancel(threads[i]);
  }
  
  for (int i = 0; i < N; ++i) {
    pthread_join(threads[i], NULL);
  }

  return 0;
}
