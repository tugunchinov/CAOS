#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>

int
main(int argc, char** argv) {
  char data[BUFSIZ];
  memset(data, 0, sizeof(data));
  
  int byte = 0;
  size_t i = 0;
  while ((byte = getchar()) != EOF) {
    data[i++] = byte;
  }

  unsigned char hash[SHA512_DIGEST_LENGTH];
  SHA512(data, strlen(data), hash);

  printf("0x");
  for (size_t i = 0; i < SHA512_DIGEST_LENGTH; ++i) {
    printf("%02x", hash[i]);
  }
  
  return 0;
}
