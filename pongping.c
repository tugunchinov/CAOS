#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

void
rt_sig_handler(int sig, siginfo_t* info, void* ucontext) {
  pid_t pid = info->si_pid;
  union sigval value = {.sival_int = info->si_int - 1};
  printf("%d\n", value.sival_int);
  fflush(stdout);
  sigqueue(pid, sig, value);
}

int
main(int argc, char** argv) {
  sigset_t mask;
  sigfillset(&mask);
  sigprocmask(SIG_BLOCK, &mask, NULL);
  struct sigaction action = {.sa_sigaction = rt_sig_handler,
                             .sa_flags = SA_RESTART|SA_SIGINFO};
  sigaction(SIGRTMIN, &action, NULL);
  sigdelset(&mask, SIGRTMIN);
  sigdelset(&mask, SIGINT);
  
  pid_t pid = 0;
  int value = 0;
  scanf("%d%d", &pid, &value);

  union sigval sigval = {.sival_int = value};
  sigqueue(pid, SIGRTMIN, sigval);
  while (1) {
    sigsuspend(&mask);
  }
  
  return 0;
}
