#include <iostream>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <vector>


void thread_routine(int number) {
  using namespace std::chrono_literals;
  while (1) {
    std::cout << "I'm thread #" << number << std::endl;
    std::cout.flush();
    std::this_thread::sleep_for(1s);
  }
}

int main(int argc, char** argv) {
  int N = std::atoi(argv[1]);

  std::vector<std::thread> threads(N);
  for (int i = 0; i < N; ++i) {
    std::thread t(thread_routine, i);
    threads[i] = std::move(t);
  }

  for (int i = 0; i < N; ++i) {
    threads[i].join();
  }

  return 0;
}
