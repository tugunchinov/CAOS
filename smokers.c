#include <stdio.h>
#include <string.h>

#include <semaphore.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

volatile sig_atomic_t sigterm = 0;

void
sigterm_handler(int sig) {
  sigterm = 1;
}

sem_t*
create_semaphore() {
  sem_t* sem = mmap(NULL,
                    sizeof(sem_t),
                    PROT_WRITE|PROT_READ,
                    MAP_SHARED|MAP_ANONYMOUS,
                    -1,
                    0);
  sem_init(sem, 1, 0);
  return sem;
}

void
destroy_semaphore(sem_t* sem) {
  munmap(sem, sizeof(sem));
}

int
main(int argc, char** argv) {
  sem_t* tsem = create_semaphore();
  sem_t* psem = create_semaphore();
  sem_t* msem = create_semaphore();
  sem_t* bsem = create_semaphore();
  
  pid_t pids[3];
  for (size_t i = 0; i < 3; ++i) {
    pids[i] = fork();
    if (pids[i] == 0) {
      struct sigaction action;
      memset(&action, 0, sizeof(struct sigaction));
      action.sa_handler = sigterm_handler;
      action.sa_flags = SA_RESETHAND;
      sigaction(SIGTERM, &action, NULL);
      
      while (!sigterm) {
        switch (i) {
          case 0:
            if (sem_wait(tsem) != -1) {
              putchar('T');
              fflush(stdout);
              sem_post(bsem);
            }
            break;
          case 1:
            if (sem_wait(psem) != -1) {
              putchar('P');
              fflush(stdout);
              sem_post(bsem);
            }
            break;
          case 2:  
            if (sem_wait(msem) != -1) {
              putchar('M');
              fflush(stdout);
              sem_post(bsem);
            }
            break;
          default:
            break;
        }
      }
      return 0;
    }
  }

  int item;
  while ((item = getchar()) != EOF) {
    switch (item) {
      case 't':
        sem_post(tsem);
        break;
      case 'p':
        sem_post(psem);
        break;
      case 'm':
        sem_post(msem);
        break;
      default:
        continue;
    }
    sem_wait(bsem);
  }

  for (size_t i = 0; i < 3; ++i) {
    kill(pids[i], SIGTERM);
  }
  
  destroy_semaphore(tsem);
  destroy_semaphore(psem);
  destroy_semaphore(msem);
  destroy_semaphore(bsem);
  
  for (size_t i = 0; i < 3; ++i) {
    wait(NULL);
  }

  return 0;
}
