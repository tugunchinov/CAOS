#include <stdio.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>

int
main(int argc, char** argv) {
  char sem_name[BUFSIZ];
  char shm_name[BUFSIZ];
  size_t N = 0;
  
  scanf("%s", sem_name);
  scanf("%s", shm_name);
  scanf("%lu", &N);
  
  sem_t* sem = sem_open(sem_name, 0);
  sem_wait(sem);
  sem_close(sem);
  
  int sfd = shm_open(shm_name, O_RDONLY, 0);
  int* array = mmap(NULL, N*sizeof(int), PROT_READ, MAP_SHARED, sfd, 0);
  close(sfd);
  
  for (size_t i = 0; i < N; ++i) {
    printf("%d\n", array[i]);
  }

  shm_unlink(shm_name);
  munmap(array, N*sizeof(int));
  
  return 0;
}
