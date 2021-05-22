#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <unistd.h>

volatile sig_atomic_t alarm_signal = 0;

typedef struct icmp_packet {
  struct icmphdr header;
  char data[64];
} icmp_packet_t;

void
alarm_handler(int sig) {
  alarm_signal = 1;
}

unsigned short
checksum(icmp_packet_t* pack, int len) {
  unsigned short* buf = (unsigned short*) pack;
  size_t sum = 0;

  for (; len > 1; len -= 2) {
    sum += *buf++;
  }
  if (len == 1) {
    sum += *(unsigned char*) buf;
  }
  sum = (sum >> 16) + (sum&0xFFFF);
  sum += (sum >> 16);
  return ~sum;
}

int
main(int argc, char** argv) {
  const struct sigaction alarm_action = {.sa_handler = alarm_handler};
  sigaction(SIGALRM, &alarm_action, NULL);
  
  const char* ipv4 = argv[1];
  const size_t timeout = strtoul(argv[2], NULL, 10);
  const size_t interval = strtoul(argv[3], NULL, 10);

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(0);
  inet_aton(ipv4, &addr.sin_addr);
  
  int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  icmp_packet_t packet;
  size_t responses = 0;
  
  alarm(timeout);
  while (!alarm_signal) {
    memset(&packet, 0, sizeof(icmp_packet_t));
    packet.header.type = ICMP_ECHO;
    packet.header.un.echo.id = getpid();
    strcpy(packet.data, "Hello!");
    packet.header.checksum = checksum(&packet, sizeof(icmp_packet_t));

    int success = 0;
    if (sendto(sockfd,
               &packet,
               sizeof(icmp_packet_t),
               0,
               (const struct sockaddr*) &addr,
               sizeof(struct sockaddr_in)) > 0) {
      success = 1;
    }
    if (success) {
      if (recvfrom(sockfd,
                   &packet,
                   sizeof(icmp_packet_t),
                   0,
                   NULL,
                   0) > 0) {
        if (packet.header.code == 0) {
            ++responses;
        }
      }
    }
    usleep(interval);
  }

  printf("%lu\n", responses);
  
  close(sockfd);
  
  return 0;
}
