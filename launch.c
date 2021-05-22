#include <inttypes.h>
#include <stdio.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

int
main(int argc, char** argv) {
  int cmd_count = argc - 1;

  int pipes[2][2];
  pid_t cmd1_pid;
  pid_t cmd2_pid;

  pipe(pipes[0]);
  cmd1_pid = fork();
  if (cmd1_pid != 0) {
    close(pipes[0][1]);
    for (int i = 2; i < cmd_count; ++i) {
      pipe(pipes[1 - i%2]);
      cmd2_pid = fork();
      if (cmd2_pid == 0) {
        dup2(pipes[i%2][0], 0);
        dup2(pipes[1 - i%2][1], 1);
        close(pipes[i%2][0]);
        close(pipes[1 - i%2][1]);
        execlp(argv[i], argv[i], NULL);
        perror("execlp");
      } else {
        close(pipes[1 - i%2][1]);
        close(pipes[i%2][0]);
        int status;
        waitpid(cmd1_pid, &status, 0);
        cmd1_pid = cmd2_pid;
      }
    }
  } else {
    dup2(pipes[0][1], 1);
    close(pipes[0][0]);
    close(pipes[0][1]);
    execlp(argv[1], argv[1], NULL);
    perror("execlp");
  }

  // last command
  cmd2_pid = fork();
  if (cmd2_pid == 0) {
    dup2(pipes[cmd_count%2][0], 0);
    close(pipes[cmd_count%2][0]);
    execlp(argv[cmd_count], argv[cmd_count], NULL);
    perror("execlp");
  } else {
    close(pipes[cmd_count%2][0]);
    int status;
    waitpid(cmd1_pid, &status, 0);
    waitpid(cmd2_pid, &status, 0);
  }
  
  return 0;
}
