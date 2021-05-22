#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <netdb.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int
main(int argc, char** argv) {
  const char* host = argv[1];
  const char* path_to_script = argv[2];
  const char* path_to_file = argv[3];
  const size_t port = 80;
  
  struct stat st;
  stat(path_to_file, &st);
  int file_size = st.st_size;
  
  const size_t request_len =
    strlen(host) + strlen(path_to_script) + strlen(path_to_file) + 256;
  char request[request_len];
  snprintf(request,
           request_len,
           "POST %s HTTP/1.1\r\n"
           "Host: %s\r\n"
           "Content-Type: multipart/form-data\r\n"
           "Connection: close\r\n"
           "Content-Length: %d\r\n\r\n",
           path_to_script,
           host,
           file_size);
  
  struct addrinfo* addrinfo = NULL;
  getaddrinfo(host, NULL, NULL, &addrinfo);
  
  struct sockaddr_in* server_addr = (struct sockaddr_in*) addrinfo->ai_addr;
  server_addr->sin_family = AF_INET;
  server_addr->sin_port = htons(port);
  
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  connect(sockfd, (struct sockaddr*) server_addr, addrinfo->ai_addrlen);
  
  write(sockfd, request, strlen(request));

  int fd = open(path_to_file, O_RDONLY);
  sendfile(sockfd, fd, NULL, file_size);
  write(sockfd, "\r\n\r\n", 4);
  close(fd);
  
  char response[8192];
  memset(response, 0, sizeof(response));
  int bytes_read = read(sockfd, response, sizeof(response));
  char* content = strstr(response, "\r\n\r\n") + 4; 
  write(1, content, bytes_read - (content - response));
  while ((bytes_read = read(sockfd, response, sizeof(response))) > 0) {
    write(1, response, bytes_read);
  }
  
  close(sockfd);
  
  return 0;
}
