#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t end = 0;

void
rt_sig_handler(int sig, siginfo_t* info, void* ucontext) {
  pid_t pid = info->si_pid;
  union sigval value = {.sival_int = info->si_int - 1};
  if (value.sival_int < 0) {
    end = 1;
    return;
  }
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

  while (!end) {
    sigsuspend(&mask);
  }
  
  return 0;
}
