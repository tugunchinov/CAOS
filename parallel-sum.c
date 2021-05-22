#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

void*
thread_routine(void* arg) {
  int* sum = calloc(1, sizeof(int));
  int value;
  while (scanf("%d", &value) > 0) {
    *sum += value;
  }
  return sum;
}

int
main(int argc, char** argv) {
  size_t threads_count = strtoul(argv[1], NULL, 10);
  
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
  pthread_attr_setguardsize(&attr, 0);

  pthread_t* threads = calloc(threads_count, sizeof(pthread_t));
  for (size_t i = 0; i < threads_count; ++i) {
    pthread_create(&threads[i], &attr, thread_routine, NULL);
  }
  pthread_attr_destroy(&attr);

  int sum = 0;
  for (size_t i = 0; i < threads_count; ++i) {
    int* current = NULL;
    pthread_join(threads[i], (void**)&current);
    sum += *current;
    free(current);
  }

  printf("%d\n", sum);
  
  free(threads);
  
  return 0;
}
