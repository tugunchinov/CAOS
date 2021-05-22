#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int
main(int argc, char** argv) {
  const char* host = argv[1];
  const char* path_to_file = argv[2];
  const int port = 80;
  
  const size_t request_len = strlen(host) + strlen(path_to_file) + 60;
  char request[request_len];
  snprintf(request,
           request_len,
           "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
           path_to_file,
           host);

  struct addrinfo* addrinfo = NULL;
  getaddrinfo(host, NULL, NULL, &addrinfo);
  
  struct sockaddr_in* server_addr = (struct sockaddr_in*) addrinfo->ai_addr;
  server_addr->sin_family = AF_INET;
  server_addr->sin_port = htons(port);
  
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  connect(sockfd, (struct sockaddr*) server_addr, addrinfo->ai_addrlen);

  write(sockfd, request, strlen(request));

  char response[256];
  memset(response, 0, sizeof(response));
  int bytes_read =  0;
  while ((bytes_read = read(sockfd, response, sizeof(response)))) {
    
  }
  char* content = strstr(response, "\r\n\r\n") + 4; 
  write(1, content, bytes_read - (content - response));
  while ((bytes_read = read(sockfd, response, sizeof(response))) > 0) {
    write(1, response, bytes_read);
  }
  
  close(sockfd);
  
  return 0;
}
