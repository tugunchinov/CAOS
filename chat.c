#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

const size_t SIZE = 4096;

typedef struct {
  char buffer[1000];
  sem_t* semaphore;
} shared_memory_t;

int
main(int argc, char** argv) {
  const char* name = argv[1];
  const char* word = argv[2];

  int fd = shm_open(name, O_RDWR|O_CREAT, 0600);
  if (fd == -1) {
    perror("open");
    return 1;
  }
  ftruncate(fd, 4096);

  shared_memory_t* memory = mmap(NULL,
                                 SIZE,
                                 PROT_READ|PROT_WRITE,
                                 MAP_SHARED,
                                 fd,
                                 0);
  if (memory == MAP_FAILED) {
    perror("mmap");
    return 2;
  }

  memory->semaphore = sem_open(name, O_CREAT, 0666, 1);
  while (1) {
    sem_wait(memory->semaphore);
    memset(memory->buffer, 0, sizeof(memory->buffer));
    strncpy(memory->buffer, word, sizeof(memory->buffer));
    printf("%s\n", memory->buffer);
    sem_post(memory->semaphore);
  }

  munmap(memory, SIZE);

  return 0;
}
