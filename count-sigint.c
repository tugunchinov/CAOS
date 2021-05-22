#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <sched.h>
#include <unistd.h>

volatile sig_atomic_t int_signal = 0;
volatile sig_atomic_t term_signal = 0;
volatile sig_atomic_t sigusr1_signal = 0;
volatile sig_atomic_t sigusr2_signal = 0;

int value = 0;

void
int_handler(int sig) {
  int_signal = 1;
}

void
term_handler(int sig) {
  term_signal = 1;
}

void
sigusr1_handler(int sig) {
  sigusr1_signal = 1;
}

void
sigusr2_handler(int sig) {
  sigusr2_signal = 1;
}

int
main(int argc, char** argv) {
  sigset_t mask;
  sigfillset(&mask);
  sigdelset(&mask, SIGINT);
  sigdelset(&mask, SIGTERM);
  sigdelset(&mask, SIGUSR1);
  sigdelset(&mask, SIGUSR2);
  sigprocmask(SIG_SETMASK, &mask, NULL);
  
  const struct sigaction int_action = {
    .sa_handler = int_handler,
    .sa_flags   = SA_RESTART,
  };
  sigaction(SIGINT, &int_action, NULL);

  const struct sigaction term_action = {
    .sa_handler = term_handler,
    .sa_flags   = SA_RESTART,
  };
  sigaction(SIGTERM, &term_action, NULL);
  
  const struct sigaction sigusr1_action = {
    .sa_handler = sigusr1_handler,
    .sa_flags   = SA_RESTART,
  };
  sigaction(SIGUSR1, &sigusr1_action, NULL);

  const struct sigaction sigusr2_action = {
    .sa_handler = sigusr2_handler,
    .sa_flags   = SA_RESTART,
  };
  sigaction(SIGUSR2, &sigusr2_action, NULL);

  pid_t pid = getpid();
  printf("%d\n", pid);
  fflush(stdout);

  scanf("%d", &value);
  while (1) {
    pause();
    if (sigusr1_signal == 1) {
      sigusr1_signal = 0;
      ++value;
      printf("%d\n", value);
      fflush(stdout);
    }
    if (sigusr2_signal == 1) {
      sigusr2_signal = 0;
      value *= -1;
      printf("%d\n", value);
      fflush(stdout);
    }
    
    if (term_signal == 1 || int_signal == 1) {
      exit(0);
    }
  }
  
  return 0;
}
