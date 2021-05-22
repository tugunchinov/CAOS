#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

volatile sig_atomic_t sig_alarm = 0;
volatile sig_atomic_t sig_child = 0;

void
alarm_handler(int sig) {
  sig_alarm = 1;
}

void
child_handler(int sig) {
  sig_child = 1;
}

int
main(int argc, char** argv) {
  sigset_t mask;
  sigfillset(&mask);
  sigprocmask(SIG_BLOCK, &mask, NULL);
  sigdelset(&mask, SIGALRM);
  sigdelset(&mask, SIGCHLD);
  
  struct sigaction alarm_action = {.sa_handler = alarm_handler};
  sigaction(SIGALRM, &alarm_action, NULL);

  struct sigaction child_action = {.sa_handler = child_handler};
  sigaction(SIGCHLD, &child_action, NULL);

  unsigned seconds = strtoul(argv[1], NULL, 10);
  
  pid_t pid = fork();
  if (pid == 0) {
    sigfillset(&mask);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    execvp(argv[2], argv + 2);
    perror("execvp");
  } else {
    alarm(seconds);
    sigsuspend(&mask);
    if (sig_alarm == 1) {
      kill(pid, SIGTERM);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    if (!sig_alarm && WIFSIGNALED(status)) {
      puts("signaled");
      return 1;
    }
  }
  if (sig_alarm) {
    puts("timeout");
    return 2;
  }
  
  puts("ok");
  return 0;
}
