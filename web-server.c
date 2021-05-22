#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

const unsigned PORT = 43225;

void
handle_error(const char* msg) {
  perror(msg);
  exit(1);
}

int
main(int argc, char** argv) {
  int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_socket == -1) {
    handle_error("listen_socket");
  }

  struct sockaddr_in listen_addr;
  memset(&listen_addr, 0, sizeof(struct sockaddr_in));
  listen_addr.sin_family = AF_INET;
  listen_addr.sin_port = htons(PORT);
  inet_aton("127.0.0.1", &listen_addr.sin_addr);
  if (bind(listen_socket,
           (struct sockaddr*) &listen_addr,
           sizeof(listen_addr)) == -1) {
    handle_error("bind");
  }
  if (listen(listen_socket, 128) == -1) {
    handle_error("listen");
  }

  // TODO: check errors
  printf("Serving HTTP on port %d\n\n", PORT);
  int client_socket = accept(listen_socket, NULL, NULL);
  while (1) {
    int value;
    if (recv(client_socket, &value, sizeof(int), 0) > 0) {
      printf("%d\n", value);
    }
    //const char http_response[] = "HTTP/1.1 200 OK\n\nHello, World!\n";
    send(client_socket, &value, sizeof(value), 0);
    // TODO: shutdown?
    //close(client_socket);
  }

  return 0;
}
