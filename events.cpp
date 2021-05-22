#include <iostream>
#include <functional>
#include <vector>
#include <string>

extern "C" {
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>
}

using Callback = std::function<void(const std::string&)>;

void printContent(const std::string& file_name, const std::string& content) {
  std::cout << "Got from " << file_name << ": " << content << std::endl;
  std::cout.flush();
}

struct EventData {
  Callback callback;
  int fd;
};

class EventLoop {
 public:
  explicit EventLoop(int epoll_size = 1024);
  void RegisterReadReadyEvent(int fd, const Callback& callback);
  void Run();

 private:
  std::vector<EventData> registers_;
  int epoll_fd_;
};

int main(int argc, char** argv) {
  EventLoop event_loop;
  for (int i = 1; i < argc; ++i) {
    const std::string file_name{argv[i]};
    int fd = open(argv[i], O_RDONLY|O_NONBLOCK);
    std::function<void(const std::string&, const std::string&)>
        callback = &printContent;
    std::function<void(const std::string&)>
        binded_callback = std::bind(callback, file_name, std::placeholders::_1);
    event_loop.RegisterReadReadyEvent(fd, binded_callback);
  }
  event_loop.Run();
  return 0;
}

EventLoop::EventLoop(int epoll_size): epoll_fd_(epoll_create(epoll_size)) {
  registers_.reserve(10000);
}

void EventLoop::RegisterReadReadyEvent(int fd, const Callback& callback) {
  EventData& evt = registers_.emplace_back(EventData{callback, fd});
  epoll_event event{};
  event.data.ptr = &evt;
  event.events = EPOLLIN;
  epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event);
}

void EventLoop::Run() {
  static constexpr int EVENTS_MAX = 4096;
  struct epoll_event events[EVENTS_MAX]{};

  int N = 0;
  while (true) {
    N = epoll_wait(epoll_fd_, events, EVENTS_MAX, -1);
    for (int i = 0; i < N; ++i) {
      epoll_event event = events[i];
      if (event.events&EPOLLIN) {
        EventData* event_data = static_cast<EventData*>(event.data.ptr);
        char buffer[4096]{};
        int count = read(event_data->fd, buffer, sizeof(buffer));
        if (count > 0) {
          event_data->callback(std::string(buffer));
        }
      }
    }
  }
}
