#include <signal.h>
#include <sched.h>
#include <stdio.h>

void
handler(int signum) {
  printf("Caught signal %d\n", signum);
}


int
main(int argc, char** argv) {
  signal(SIGINT, handler);
  signal(SIGTERM, handler);
  while (1) {
    sched_yield();
  }
  return 0;
}
