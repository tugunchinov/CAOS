#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

volatile sig_atomic_t was_sighup = 0;

void
sighup_handler(int sig) {
  was_sighup = 1;
}

int
main(int argc, char** argv) {
  struct sigaction sigpipe_action = {.sa_handler = sighup_handler,
                                     .sa_flags = SA_RESTART};
  sigaction(SIGHUP, &sigpipe_action, NULL);

  pid_t pid = getpid();
  printf("My pid = %d\n", pid);
  
  const char* fifo_name = argv[1];

  char buffer[1024];
  memset(buffer, 0, sizeof(buffer));
  pause();
  if (was_sighup) {
    printf("Caught signal.\n");
    fflush(stdout);
    int fd = open(fifo_name, O_RDONLY);
    close(fd);
    return 0;
    if (fd == -1) {
      perror("open");
      return 1;
    }

    int count = 0;
    while (read(fd, buffer, sizeof(buffer)) > 0) {
      ++count;
      if (count == 5) {
	close(fd);
	break;
      }
      puts(buffer);
      memset(buffer, 0, sizeof(buffer));
    }
    close(fd);
    puts("");
    puts(buffer);
  }

  return 0;
}
