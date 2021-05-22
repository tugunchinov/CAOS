#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>


typedef struct Item {
  struct Item* _Atomic next;
  int64_t value;
} item_t;

typedef struct {
  item_t* _Atomic head;
  size_t index;
  size_t items_count;
} thread_data_t;

void
insert(item_t* _Atomic head, int64_t value) {
  item_t* new_node = calloc(1, sizeof(item_t));
  new_node->value = value;
  item_t* _Atomic current = head;
  while (1) {
    while (current->next) {
      current = current->next;
    }
    item_t* expected = NULL;
    if (atomic_compare_exchange_strong(&current->next, &expected, new_node)) {
      break;
    }
    sched_yield();
  }
}

void*
thread_routine(void* arg) {
  thread_data_t* data = arg;
  int64_t value = data->index*data->items_count;
  for (size_t i = 0; i < data->items_count; ++i) {
    insert(data->head, value + i);
  }
  return NULL;
}

int
main(int argc, char** argv) {
  size_t threads_count = strtoul(argv[1], NULL, 10);
  size_t items_count = strtoul(argv[2], NULL, 10);

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
  pthread_attr_setguardsize(&attr, 0);

  item_t* _Atomic head = calloc(1, sizeof(item_t));
  
  pthread_t* threads = calloc(threads_count, sizeof(pthread_t));
  thread_data_t* data = calloc(threads_count, sizeof(thread_data_t));
  for (size_t i = 0; i < threads_count; ++i) {
    atomic_store(&data[i].head, head);
    data[i].items_count = items_count;
    data[i].index = i;
    pthread_create(&threads[i], &attr, thread_routine, &data[i]);
  }
  pthread_attr_destroy(&attr);

  for (size_t i = 0; i < threads_count; ++i) {
    pthread_join(threads[i], NULL);
  }

  item_t* current = head->next;
  free(head);
  while (current) {
    item_t* deleted = current;
    printf("%ld\n", current->value);
    current = current->next;
    free(deleted);
  }
  
  free(threads);
  free(data);
  
  return 0;
}
