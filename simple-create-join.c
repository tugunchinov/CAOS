#include <limits.h>
#include <stdio.h>

#include <pthread.h>

void*
thread_routine(void* arg) {
  pthread_attr_t* attr = (pthread_attr_t*) arg;
  int value;
  if (scanf("%d", &value) > 0) {
    pthread_t thread;
    pthread_create(&thread, attr, thread_routine, attr);
    pthread_join(thread, NULL);
    printf("%d\n", value);
  }
  return NULL;
}

int
main(int argc, char** argv) {
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
  pthread_attr_setguardsize(&attr, 0);

  pthread_t thread;
  pthread_create(&thread, &attr, thread_routine, &attr);
  pthread_join(thread, NULL);
  
  pthread_attr_destroy(&attr);
  
  return 0;
}
