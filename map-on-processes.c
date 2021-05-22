#include <stdlib.h>
#include <string.h>

#include <semaphore.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <unistd.h>

typedef double (*function_t)(double);

double*
pmap_process(function_t func, const double *in, size_t count) {
  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = SIG_IGN;
  action.sa_flags = SA_RESTART;
  sigaction(SIGCHLD, &action, NULL);

  double* shmem = mmap(NULL,
                       sizeof(double)*count,
                       PROT_WRITE|PROT_READ,
                       MAP_SHARED|MAP_ANONYMOUS,
                       -1,
                       0);

  sem_t* sem = mmap(NULL,
                    sizeof(sem_t),
                    PROT_WRITE|PROT_READ,
                    MAP_SHARED|MAP_ANONYMOUS,
                    -1,
                    0);
  sem_init(sem, 1, 0);

  
  size_t proc_count = get_nprocs();
  for (size_t i = 0; i < proc_count; ++i) {
    pid_t pid = fork();
    if (pid == 0) {
      for (size_t j = i; j < count; j += proc_count) {
        shmem[j] = func(in[j]);
      }
      sem_post(sem);
      exit(0);
    }
  }

  for (size_t i = 0; i < proc_count; ++i) {
    sem_wait(sem);
  }

  for (size_t i = 0; i < proc_count; ++i) {
    wait(NULL);
  }
  
  munmap(sem, sizeof(sem_t));
  
  return shmem;
}


void
pmap_free(double *ptr, size_t count) {
  munmap(ptr, sizeof(double)*count);
}
