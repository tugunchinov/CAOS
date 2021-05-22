#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

void
build_matrix(char* mapped, int64_t N, int W) {
  for (int i = 1; i <= N; ++i) {
    mapped[i*N*W + i - 1] = '\n';
  }
  int64_t cur_num = 1;
  char str_num[W + 1];
  for (int64_t i = 0; i < (N + 1)/2; ++i) {
    for (int64_t j = i; j < N - i; ++j) {
      snprintf(str_num, W + 1, "%*"PRId64, W, cur_num);
      memcpy(&mapped[N*W*i + i + j*W], str_num, W);
      ++cur_num;
    }
    for (int64_t j = i + 1; j < N - i; ++j) {
      snprintf(str_num, W + 1, "%*"PRId64, W, cur_num);
      memcpy(&mapped[N*W*j + j + W*(N - i - 1)], str_num, W);
      ++cur_num;
    }
    for (int64_t j = N - i - 2; j >= i; --j) {
      snprintf(str_num, W + 1, "%*"PRId64, W, cur_num);
      memcpy(&mapped[N*W*(N - i - 1) + (N - i -1) + j*W], str_num, W);
      ++cur_num;
    }
    for (int64_t j = N - i - 2; j > i; --j) {
      snprintf(str_num, W + 1, "%*"PRId64, W, cur_num);
      memcpy(&mapped[N*W*j + j + W*i], str_num, W);
      ++cur_num;
    }
  }
}

int
main(int argc, char** argv) {
  int fd = open(argv[1], O_RDWR | O_CREAT, 0640);
  if (fd == -1) {
    perror("open");
    return 1;
  }
  int64_t N = strtol(argv[2], NULL, 10);
  int W = strtol(argv[3], NULL, 10);
  size_t file_size = N*N*W + N;
  if (ftruncate(fd, file_size) == -1) {
    perror("ftruncate");
    if (close(fd) == -1) {
      perror("close");
    }
    return 1;
  }
  void* mapped = mmap(NULL,
                      file_size,
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED,
                      fd,
                      0);

  if (close(fd) == -1) {
    perror("close");
    return 1;
  }
  if (mapped == MAP_FAILED) {
    perror("mmap");
    return 2;
  }
  build_matrix(mapped, N, W);
  if (munmap(mapped, file_size) == -1) {
    perror("munmap");
    return 2;
  }
  return 0;
}
