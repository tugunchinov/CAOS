#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void
handle_error(const char* msg) {
  perror(msg);
  exit(1);
}

int
main(int argc, char** argv) {
  const char* IPv4_ADDRESS = argv[1];
  const unsigned PORT = strtoul(argv[2], NULL, 10);
  
  int sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1) {
    handle_error("listen_socket");
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  inet_aton(IPv4_ADDRESS, &addr.sin_addr);
  if (connect(sfd,
             (struct sockaddr*) &addr,
             sizeof(struct sockaddr_in)) == -1) {
    handle_error("connect");
  }

  int value;
  scanf("%d", &value);
  for (;;) {
    if (send(sfd, &value, sizeof(int), 0) <= 0) {
      break;
    }
    if (recv(sfd, &value, sizeof(int), 0) <= 0) {
      break;
    }
    printf("%d\n", value);
    if (scanf("%d", &value) <= 0) {
      break;
    }
  }
  shutdown(sfd, SHUT_RDWR);
  close(sfd);
  return 0;
}
