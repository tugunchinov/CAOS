#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

volatile sig_atomic_t was_sigpipe = 0;

void
sigpipe_handler(int sig) {
  was_sigpipe = 1;
}

int
main(int argc, char** argv) {
  struct sigaction sigpipe_action = {.sa_handler = sigpipe_handler,
				     .sa_flags = SA_RESTART};
  sigaction(SIGPIPE, &sigpipe_action, NULL);

  const char* fifo_name = argv[1];
  const int N = atoi(argv[2]);

  if (mkfifo(fifo_name, 0666) == -1) {
    perror("mkfifo");
    return 1;
  }

  pid_t pid = 0;
  scanf("%d", &pid);
  
  kill(pid, SIGHUP);
  int fd = open(fifo_name, O_WRONLY);
  if (fd == -1) {
    perror("open");
    return 1;
  }
  
  char num[64];
  int nums_written = 0;
  for (int i = 0; i <= N; ++i) {
    snprintf(num, sizeof(num), "%d ", i);
    write(fd, num, strlen(num));
    if (was_sigpipe) {
      break;
    }
    ++nums_written;
  }
  close(fd);
  printf("%d\n", nums_written);
  
  return 0;
}
