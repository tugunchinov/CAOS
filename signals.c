#include <stdio.h>

#include <signal.h>
#include <unistd.h>

sig_atomic_t int_count = 0;
sig_atomic_t term_count = 0;

void
term_handler(int sig) {
  ++term_count;
}

void
int_handler(int sig, siginfo_t* info, void* ctx) {
  //int_count += info->si_int;
  ++int_count;
}

int
main(int argc, char** argv) {
  sigaction(SIGRTMIN, &(struct sigaction){.sa_flags = SA_RESTART|SA_SIGINFO,
					.sa_sigaction = int_handler}, NULL);
  sigaction(SIGTERM, &(struct sigaction){.sa_flags = SA_RESTART, .sa_handler = term_handler}, NULL);
  printf("PID = %d\n", getpid());
  fflush(stdout);

  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGRTMIN);

  sigprocmask(SIG_BLOCK, &mask, NULL);
  
  while (!term_count) {
    pause();
  }

  sigprocmask(SIG_UNBLOCK, &mask, NULL);
  
  printf("sum(INT) = %d, count(TERM) = %d\n", int_count, term_count);
  fflush(stdout);
  return 0;
}
