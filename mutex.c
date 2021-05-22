#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

typedef struct {
  double* value;
  size_t index;
  size_t size;
  size_t iterations_count;
  pthread_mutex_t* mutex;
} thread_data_t;

void*
thread_routine(void* thread_data) {
  thread_data_t* data = thread_data;
  size_t current = data->index;
  size_t left = current > 0 ? current - 1 : data->size - 1;
  size_t right = current < data->size - 1 ? current + 1 : 0;
  for (size_t i = 0; i < data->iterations_count; ++i) {
    pthread_mutex_lock(data->mutex);
    data->value[current] += 1.0;
    data->value[left] += 0.99;
    data->value[right] += 1.01;
    pthread_mutex_unlock(data->mutex);
  }
  return NULL;
}

int
main(int argc, char** argv) {
  size_t iterations_count = strtoul(argv[1], NULL, 10);
  size_t threads_count = strtoul(argv[2], NULL, 10);
  double* values = calloc(threads_count, sizeof(double));

  pthread_t* threads = calloc(threads_count, sizeof(pthread_t));
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  thread_data_t* data = calloc(threads_count, sizeof(thread_data_t));
  for (size_t i = 0; i < threads_count; ++i) {
    data[i].value = values;
    data[i].index = i;
    data[i].size = threads_count;
    data[i].iterations_count = iterations_count;
    data[i].mutex = &mutex;
    pthread_create(&threads[i], NULL, thread_routine, &data[i]);
  }
  
  for (size_t i = 0; i < threads_count; ++i) {
    pthread_join(threads[i], NULL);
  }

  for (size_t i = 0; i < threads_count; ++i) {
    printf("%.10g\n", values[i]);
  }

  pthread_mutex_destroy(&mutex);
  
  free(values);
  free(threads);
  free(data);
  
  return 0;
}
