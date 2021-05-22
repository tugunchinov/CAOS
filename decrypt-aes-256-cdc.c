#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include <openssl/evp.h>
#include <unistd.h>

void
read_salt(unsigned char* salt) {
  read(0, salt, 8);
  read(0, salt, 8);
}

void
decrypt(unsigned char* key, unsigned char* iv) {
  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    
  EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

  unsigned char text[4*BUFSIZ];
  memset(text, 0, sizeof(text));
  
  unsigned char encrypted[4*BUFSIZ];
  int bytes_read = read(0, encrypted, sizeof(encrypted));
  int len = 0;
  EVP_DecryptUpdate(ctx, text, &len, encrypted, bytes_read);
  int text_len = len;
  EVP_DecryptFinal_ex(ctx, text + len, &len);
  text_len += len;

  write(1, text, text_len);
  
  EVP_CIPHER_CTX_free(ctx);
}

int
main(int argc, char** argv) {
  const char* password = argv[1];
  
  unsigned char salt[8];
  read_salt(salt);
  
  OpenSSL_add_all_algorithms();
  
  const EVP_CIPHER* cipher = EVP_aes_256_cbc();
  const EVP_MD* dgst = EVP_sha256();

  unsigned char key[EVP_MAX_KEY_LENGTH];
  unsigned char iv[EVP_MAX_IV_LENGTH];

  EVP_BytesToKey(cipher,
                 dgst,
                 salt,
                 (const unsigned char*) password,
                 strlen(password),
                 1,
                 key,
                 iv);
  
  decrypt(key, iv);
  
  return 0;
}
