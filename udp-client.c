#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int
main(int argc, char** argv) {
  const size_t port = strtoul(argv[1], NULL, 10);

  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  inet_aton("127.0.0.1", &server_addr.sin_addr);
  
  int num = 0;
  while (scanf("%d", &num) != EOF) {
    sendto(sockfd,
           &num,
           sizeof(int),
           0,
           (const struct sockaddr*) &server_addr,
           sizeof(server_addr));
    recvfrom(sockfd,
             &num,
             1024,
             0,
             NULL,
             0);
    printf("%d\n", num);
  }

  close(sockfd);
  
  return 0;
}
