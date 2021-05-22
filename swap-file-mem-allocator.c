#include <inttypes.h>
#include <stdio.h> 
#include <string.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static struct {
  void* ptr;
  size_t size;
} swap_file;

typedef struct MemBlock {
  struct MemBlock* next;
  struct MemBlock* prev;
  int64_t size;
  _Bool free;
} __attribute__((__packed__))
MemBlock;

extern void
myalloc_initialize(int fd) {
  struct stat st;
  if (fstat(fd, &st) == -1) {
    perror("fstat");
    return;
  }
  swap_file.size = st.st_size;
  swap_file.ptr = mmap(NULL,
                       st.st_size,
                       PROT_READ | PROT_WRITE,
                       MAP_SHARED,
                       fd,
                       0);
  if (swap_file.ptr == MAP_FAILED) {
    perror("mmap");
    return;
  }
  MemBlock init = {NULL, NULL, st.st_size - sizeof(MemBlock), 1};
  memcpy(swap_file.ptr, &init, sizeof(MemBlock));
}

extern void
myalloc_finalize() {
  if (munmap(swap_file.ptr, swap_file.size) == -1) {
    perror("munmap");
  }
}

extern void*
my_malloc(size_t size) {
  if (swap_file.ptr == MAP_FAILED) {
    return NULL;
  }
  MemBlock* cur = swap_file.ptr;
  while (!cur->free || cur->size < size) {
    if (cur->next == NULL) {
      return NULL;
    }
    cur = cur->next;
  }
  MemBlock new;
  new.free = 1;
  if (cur->next != NULL) {
    new.size = ((char*)cur->next - (char*)cur) - (size + 2*sizeof(MemBlock));
    new.next = cur->next;
  } else {
    new.size = cur->size - size - sizeof(MemBlock);
    new.next = NULL;
  }
  cur->free = 0;
  if (new.size > 0) { 
    cur->size = size;
    cur->next = (MemBlock*)((char*)cur + sizeof(MemBlock) + size);
    new.prev = cur;
    memcpy(cur->next, &new, sizeof(MemBlock));
  }
  return ++cur;
}

extern void
my_free(void* ptr) {
  if (ptr == NULL) {
    return;
  }
  MemBlock* block = ptr;
  --block;
  block->free = 1;
  MemBlock* tmp = block;
  while (tmp->next && tmp->next->free) {
    tmp = tmp->next;
  }
  if (tmp != block) {
    block->size += (char*)tmp - (char*)block + tmp->size;
    block->next = tmp->next;
  }
  if (block->prev != NULL) {
    block->prev->next = block->next;
    block->prev->size += sizeof(MemBlock) + block->size;
  }
}
