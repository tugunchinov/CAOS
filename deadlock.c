#include <sys/types.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

enum { DataSize = 256 };

int
main(int argc, char** argv) {
  int fds[2];
  pipe(fds);
  
  pid_t reader_pid;
  if ((reader_pid = fork()) == 0) {
    close(fds[1]);
    char buffer[4096];
    ssize_t count;
    while ((count = read(fds[0], buffer, sizeof(buffer))) > 0) {
      write(1, buffer, count);
    }
    puts("\nReader done");
    exit(0);
  }

  

  pid_t writer_pid;
  if ((writer_pid = fork()) == 0) {
    close(fds[0]);
    for (int i = 0; i < DataSize; ++i) {
      char letter[1];
      letter[0] = 'A' + (i%26);
      write(fds[1], letter, sizeof(letter));
    }
    puts("\nWriter done");
    exit(0);
  }


  close(fds[0]);
  //close(fds[1]);
  
  wait(NULL);
  wait(NULL);

  return 0;
}
