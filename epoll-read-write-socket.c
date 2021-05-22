#define _GNU_SOURCE

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


const static size_t CAPACITY = 4096;

typedef struct {
  char read_end;
  int sd;
  char* data;
  size_t size;
  size_t index;
} response_t;


volatile sig_atomic_t processing = 0;
volatile sig_atomic_t sigterm = 0;
volatile sig_atomic_t sigpipe = 0;

void
sigterm_handler(int sig) {
  sigterm = 1;
}

void
sigpipe_handler(int sig) {
  sigpipe = 1;
}

void
handle_error(const char* msg) {
  perror(msg);
  exit(1);
}


// doesn't expect '\0'
void
to_upper(size_t N, char str[N]) {
  for (size_t i = 0; i < N; ++i) {
    str[i] = toupper(str[i]);
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
register_event(int epoll, int sd) {
  response_t* response = calloc(1, sizeof(response_t));
  response->sd = sd;
  response->data = calloc(CAPACITY, sizeof(char));
  struct epoll_event event = {.events = EPOLLIN,
                              .data = {.ptr = response}};
  epoll_ctl(epoll, EPOLL_CTL_ADD, sd, &event);
}

void
destroy_client_socket(int sd, response_t* response) {
  free(response->data);
  free(response);
  response = NULL;
  close(sd);
}

void
run_server(int epoll, int server_socket) {
  struct epoll_event events[256];
  while (!sigterm) {
    int ready = epoll_wait(epoll, events, 256, -1);
    if (sigterm) {
      break;
    }

    for (int i = 0; i < ready; ++i) {
      response_t* response = (response_t*) events[i].data.ptr;
      // New client
      if (response->sd == server_socket) {
        int client_socket = accept4(server_socket, NULL, NULL, SOCK_NONBLOCK);
        register_event(epoll, client_socket);
      } else {
        // Reading
        ssize_t nbytes;
        if (events[i].events & EPOLLIN) {
          if ((nbytes = read(response->sd, response->data, CAPACITY)) == 0) {
            // Client's socket is closed
            destroy_client_socket(response->sd, response);
            continue;
          } else if (nbytes > 0) {
            response->size = nbytes;
            response->index = 0;
            to_upper(response->size, response->data);
          }
        }
        // Writing
        if ((nbytes = write(response->sd,
                            response->data +
                            response->index,
                            response->size)) > 0) {
          response->size -= nbytes;
          response->index += nbytes;
          if (response->size == 0) {
            // If is in epoll queue, leave it
            events[i].events = EPOLLIN;
            epoll_ctl(epoll, EPOLL_CTL_MOD, response->sd, &events[i]);
          }
        } else if (response->size > 0) {
          // Has something to write, get in queue 
          events[i].events = EPOLLOUT;
          epoll_ctl(epoll, EPOLL_CTL_MOD, response->sd, &events[i]);
        }
        // Client socket is closed
        if (sigpipe) {
          sigpipe = 0;
          destroy_client_socket(response->sd, response);
        }
      }
    }
  }
}

int
main(int argc, char** argv) {
  set_signal_handler(SIGTERM, sigterm_handler);
  set_signal_handler(SIGPIPE, sigpipe_handler);
  
  const unsigned PORT = strtoul(argv[1], NULL, 10);

  struct addrinfo* addrinfo = NULL;
  getaddrinfo("localhost", NULL, NULL, &addrinfo);
  struct sockaddr_in* listen_addr = (struct sockaddr_in*) addrinfo->ai_addr;
  listen_addr->sin_family = AF_INET;
  listen_addr->sin_port = htons(PORT);

  int listen_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
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

  response_t* response = calloc(1, sizeof(response_t));
  response->sd = listen_socket;
  int epoll = epoll_create(1);
  struct epoll_event event = {.events = EPOLLIN,
                              .data = {.ptr = response}};
  epoll_ctl(epoll, EPOLL_CTL_ADD, listen_socket, &event);
  run_server(epoll, listen_socket);
  shutdown(listen_socket, SHUT_RD);
  close(listen_socket);
  free(response);
  
  return 0;
}
