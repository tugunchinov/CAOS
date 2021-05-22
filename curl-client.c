#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <unistd.h>

typedef struct {
  char* data;
  size_t size;
  _Bool finished;
} buffer_t;

static size_t
callback(char* data, size_t size, size_t nmemb, buffer_t* buf) {
  if (!buf->finished) {
    memcpy(buf->data + buf->size, data, size*nmemb);
    buf->size += size*nmemb;
    char* e_ptr = strstr(buf->data, "</title>");
    if (e_ptr) {
      char* s_ptr = strstr(buf->data, "<title>");
      s_ptr += sizeof("<title>") - 1;
      write(1, s_ptr, e_ptr - s_ptr);
      buf->finished = 1;
    }
  }
  return size*nmemb;
}

int
main(int argc, char** argv) {
  const char* url = argv[1];

  char* data = calloc(20000, sizeof(char));
  buffer_t buffer = {data, 0, 0};
  
  CURL* curl = curl_easy_init();

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
  curl_easy_setopt(curl, CURLOPT_URL, url);
  CURLcode result = curl_easy_perform(curl);

  curl_easy_cleanup(curl);

  free(data);
  
  return 0;
}
