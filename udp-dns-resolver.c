#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

size_t
parse_domen(char* result, int id, const char* domen) {
  memcpy(result, &id, sizeof(int));
  result[2] = 1;
  result[5] = 1;
  char* cur = result + 12;
  char* token = calloc(strlen(domen), sizeof(char));
  int read = 0;
  int written = 0;
  while ((read = sscanf(domen, "%[^.]", token)) > 0) {
    int len = strlen(token);
    domen += len + 1;
    memcpy(cur, &len, sizeof(char));
    ++cur;
    cur += sprintf(cur, "%s", token);
  }
  free(token);
  *(cur + 2) = 1;
  *(cur + 4) = 1;
  return cur + 4 - result + 1;
}

int
main(int argc, char** argv) {
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(53);
  inet_aton("8.8.8.8", &addr.sin_addr);
  
  char domen[1024];
  char address[1024];
  int id = 0;
  while (fgets(domen, sizeof(domen), stdin)) {
    domen[strlen(domen) - 1] = 0;
    char* parsed_domen = calloc(2*strlen(domen) + 15, sizeof(char));;
    size_t len = parse_domen(parsed_domen, id, domen);
    ++id;
    sendto(sockfd,
           parsed_domen,
           len,
           0,
           (const struct sockaddr*) &addr,
           sizeof(addr));
    ssize_t recved = recvfrom(sockfd,
                              domen,
                              sizeof(domen),
                              0,
                              NULL,
                              0);
    inet_ntop(AF_INET, &domen[recved - 4], address, INET_ADDRSTRLEN);
    puts(address);
    free(parsed_domen);
  }

  close(sockfd);
  
  return 0;
}
