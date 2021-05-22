#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

volatile sig_atomic_t processing = 0;
volatile sig_atomic_t sigterm = 0;
volatile sig_atomic_t sigint = 0;

void
sigterm_handler(int sig) {
  if (!sigterm && !sigint) {
    if (!processing) {
      exit(0);
    }
    sigterm = 1;
  }
}

void
sigint_handler(int sig) {
  if (!sigterm && !sigint) {
    if (!processing) {
      exit(0);
    }
    sigint = 1;
  }
}

void
handle_error(const char* msg) {
  perror(msg);
  exit(1);
}

void
extract_file_name(char* buffer) {
  size_t len = strlen(buffer);
  for (size_t j = 0, i = 4; i < len; ++i) {
    if (buffer[i] == ' ') {
      buffer[j] = '\0';
      break;
    }
    buffer[j++] = buffer[i];
  }
}

void
set_signal_handler(int signum, void (*handler)(int)) {
  const struct sigaction action = {
    .sa_handler = handler,
    .sa_flags   = SA_RESTART,
  };
  sigaction(signum, &action, NULL);
}

void
process_request(int socket, const char* path) {
  char buffer[8192];
  memset(buffer, 0, sizeof(buffer));
  int nbytes = 0;
  char* buf_ptr = buffer;
  char finished = 0;
  while ((nbytes += recv(socket,
                         buf_ptr,
                         sizeof(buffer) - nbytes - 1,
                         0)) <= 4) {
    buf_ptr += nbytes;
  }
  if (strstr(buffer, "\r\n\r\n")) {
    finished = 1;
  }
  buf_ptr = buffer + 4;
  char* end_ptr = buffer + 4;
  do {
    while (*end_ptr != '\0' && *end_ptr != ' ') {
      ++end_ptr;
    }
    if (*end_ptr == ' ') {
      break;
    }
    nbytes += recv(socket, end_ptr, sizeof(buffer) - nbytes, 0);
  } while (1);
  extract_file_name(buffer);
  char full_path[4096];
  strcpy(full_path, path);
  strcat(full_path, "/");
  strcat(full_path, buffer);
  while (!finished && (nbytes = recv(socket,
                                     buffer,
                                     sizeof(buffer) - 1,
                                     0)) > 0) {
    if (strstr(buffer, "\r\n\r\n")) {
      finished = 1;
    } else if (nbytes == 2 && buffer[0] == '\r' && buffer[1] == '\n') {
      finished = 1;
    }
  }
  if (access(full_path, F_OK)) {
    const char* not_found_response = "HTTP/1.1 404 Not Found\r\n";
    send(socket, not_found_response, strlen(not_found_response), 0);
    const char* zero_length = "Content-Length: 0\r\n";
    send(socket, zero_length, strlen(zero_length), 0);
    send(socket, "\r\n", 2, 0);
  } else if (access(full_path, R_OK)) {
    const char* forbidden_response = "HTTP/1.1 403 Forbidden\r\n";
    send(socket, forbidden_response, strlen(forbidden_response), 0);
    const char* zero_length = "Content-Length: 0\r\n";
    send(socket, zero_length, strlen(zero_length), 0);
    send(socket, "\r\n", 2, 0);
  } else if (access(full_path, X_OK)) {
    const char* ok_response = "HTTP/1.1 200 OK\r\n";
    send(socket, ok_response, strlen(ok_response), 0);
    struct stat st;
    stat(full_path, &st);
    int size = st.st_size;
    snprintf(buffer, sizeof(buffer), "Content-Length: %d\r\n", size);
    send(socket, buffer, strlen(buffer), 0);
    send(socket, "\r\n", 2, 0);
    int fd = open(full_path, O_RDONLY);
    char file_data[1024];
    size_t nbytes = 0;
    while ((nbytes = read(fd, file_data, sizeof(file_data))) > 0) {
      send(socket, file_data, nbytes, 0);
    }
    close(fd);
  } else {
    const char* ok_response = "HTTP/1.1 200 OK\r\n";
    send(socket, ok_response, strlen(ok_response), 0);
    pid_t pid = fork();
    if (pid == 0) {
      dup2(socket, 1);
      execlp(full_path, full_path, NULL);
    } else {
      int status = 0;
      waitpid(pid, &status, 0);
    }
  }
}

int
main(int argc, char** argv) {
  set_signal_handler(SIGINT, sigint_handler);
  set_signal_handler(SIGTERM, sigterm_handler);
  
  const unsigned PORT = strtoul(argv[1], NULL, 10);
  const char* PATH = argv[2];

  struct addrinfo* addrinfo = NULL;
  getaddrinfo("localhost", NULL, NULL, &addrinfo);
  struct sockaddr_in* listen_addr = (struct sockaddr_in*) addrinfo->ai_addr;
  listen_addr->sin_family = AF_INET;
  listen_addr->sin_port = htons(PORT);

  int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_socket == -1) {
    handle_error("listen_socket");
  }
  if (bind(listen_socket,
           (struct sockaddr*) listen_addr,
           sizeof(struct sockaddr_in)) == -1) {
    handle_error("bind");
  }
  if (listen(listen_socket, 128) == -1) {
    handle_error("listen");
  }

  while (!sigterm && !sigint) {
    int client_socket = accept(listen_socket, NULL, NULL);
    processing = 1;
    process_request(client_socket, PATH);
    shutdown(client_socket, SHUT_RDWR);
    close(client_socket);
    processing = 0;
  }

  shutdown(listen_socket, SHUT_RD);
  close(listen_socket);
  
  return 0;
}
