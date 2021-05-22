#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

void
make_nonblock(size_t N, int in[N]) {
  for (size_t i = 0; i < N; ++i) {
    int flags = fcntl(in[i], F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(in[i], F_SETFL, flags);
  }
}

void
register_events(int epoll, size_t N, int in[N]) {
  for (size_t i = 0; i < N; ++i) {
    struct epoll_event event = {.events = EPOLLIN,
                                .data = {.u32 = i}};
    epoll_ctl(epoll, EPOLL_CTL_ADD, in[i], &event);
  }
}

extern size_t
read_data_and_count(size_t N, int in[N]) {
  make_nonblock(N, in);

  int epoll = epoll_create(1);
  register_events(epoll, N, in); 

  char buffer[4096];
  size_t total = 0;
  struct epoll_event events[N];
  while (N > 0) {
    int ready = epoll_wait(epoll, events, N, -1);
    for (int k = 0; k < ready; ++k) {
      size_t i = events[k].data.u32;
      ssize_t current = 0;
      while ((current = read(in[i], buffer, sizeof(buffer))) > 0) {
        total += current;
      }
      if (current == 0) {
        epoll_ctl(epoll, EPOLL_CTL_DEL, in[i], NULL);
        close(in[i]);
        --N;
      }
    }
  }
  return total;
}
