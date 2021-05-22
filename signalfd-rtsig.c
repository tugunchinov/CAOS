#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t rt_sig = 0;

void
rt_sig_handler(int sig) {
  rt_sig = sig;
}

int
main(int argc, char** argv) {
  sigset_t mask;
  sigfillset(&mask);
  sigprocmask(SIG_BLOCK, &mask, NULL);
  struct sigaction action = {.sa_handler = rt_sig_handler,
                             .sa_flags = SA_RESTART};
  for (int sig = SIGRTMIN; sig <= SIGRTMAX; ++sig) {
    sigaction(sig, &action, NULL);
    sigdelset(&mask, sig);
  }

  FILE* file[argc - 1];
  for (int i = 0; i < argc - 1; ++i) {
    file[i] = fopen(argv[i + 1], "r");
  }

  char line[4096];
  while (1) {
    sigsuspend(&mask);
    int x = rt_sig - SIGRTMIN;
    if (x == 0) {
      break;
    }
    rt_sig = 0;
    fgets(line, sizeof(line), file[x - 1]);
    printf("%s", line);
    fflush(stdout);
  }
  
  for (int i = 0; i < argc - 1; ++i) {
    fclose(file[i]);
  }
  
  return 0;
}
