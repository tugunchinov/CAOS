#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <sched.h>

typedef struct {
  uint64_t next_prime;
  uint64_t A;
  uint64_t B;
  uint32_t N;
  pthread_mutex_t* mutex;
  pthread_cond_t* has_prime;
  pthread_cond_t* printed;
} thread_data_t;

int
is_prime(uint64_t num) {
  uint64_t root = sqrt(num);
  for (uint64_t i = 2; i <= root; ++i) {
    if (num % i == 0) {
      return 0;
    }
  }
  return 1;
}

void*
thread_routine(void* thread_data) {
  thread_data_t* data = thread_data;
  uint64_t A = data->A;
  uint64_t B = data->B;
  
  pthread_mutex_lock(data->mutex);
  for (uint64_t i = A; i <= B; ++i) {
    if (data->N == 0) {
      pthread_mutex_unlock(data->mutex);
      break;
    }
    while (data->next_prime != 0) {
      pthread_cond_wait(data->printed, data->mutex);
    }
    if (is_prime(i)) {
      data->next_prime = i;
      --data->N;
      pthread_cond_signal(data->has_prime);
    }
  }
  
  return NULL;
}

int
main(int argc, char** argv) {
  uint64_t A = strtoul(argv[1], NULL, 10);
  uint64_t B = strtoul(argv[2], NULL, 10);
  uint32_t N = strtoul(argv[3], NULL, 10);

  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t has_prime = PTHREAD_COND_INITIALIZER;
  pthread_cond_t printed = PTHREAD_COND_INITIALIZER;
  
  pthread_t thread;
  thread_data_t data;
  data.next_prime = 0;
  data.A = A;
  data.B = B;
  data.N = N;
  data.mutex = &mutex;
  data.has_prime = &has_prime;
  data.printed = &printed;
  
  pthread_mutex_lock(&mutex);
  pthread_create(&thread, NULL, thread_routine, &data);

  while (data.N > 0) {
    while (data.next_prime == 0) {
      pthread_cond_wait(&has_prime, &mutex);
    }
    printf("%lu\n", data.next_prime);
    data.next_prime = 0;
    pthread_cond_signal(&printed);
  }

  pthread_mutex_unlock(&mutex);
  pthread_join(thread, NULL);

  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&has_prime);
  pthread_cond_destroy(&printed);
  
  return 0;
}
