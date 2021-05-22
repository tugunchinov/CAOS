#include <stdio.h>
#include <string.h>

#include <dlfcn.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>

typedef double (*function_t)(double);

typedef struct {
  sem_t request_ready;
  sem_t response_ready;
  char func_name[20];
  double value;
  double result;
} shared_data_t;

int
main(int argc, char** argv) {
  const char* lib_name = argv[1];
  const char* shm_name = "/tugunchinovamivyac17";

  int shm = shm_open(shm_name, O_RDWR|O_CREAT, 0666);
  ftruncate(shm, sizeof(shared_data_t));

  shared_data_t* shdata = mmap(NULL,
                               sizeof(shared_data_t),
                               PROT_WRITE|PROT_READ,
                               MAP_SHARED,
                               shm,
                               0);
  close(shm);
  sem_init(&shdata->response_ready, 1, 0);
  sem_init(&shdata->request_ready, 1, 0);

  void* lib = dlopen(lib_name, RTLD_NOW);
  puts(shm_name);
  fflush(stdout);

  sem_wait(&shdata->request_ready);
  while (strlen(shdata->func_name) > 0) {
    function_t function = dlsym(lib, shdata->func_name);
    shdata->result = function(shdata->value);
    sem_post(&shdata->response_ready);
    sem_wait(&shdata->request_ready);
  }
  
  sem_destroy(&shdata->request_ready);
  sem_destroy(&shdata->response_ready);
  munmap(shdata, sizeof(shared_data_t));
  shm_unlink(shm_name);
  dlclose(lib);

  return 0;
}
