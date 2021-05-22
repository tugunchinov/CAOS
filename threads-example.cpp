#include <iostream>
#include <thread>
#include <chrono>

void Foo() {
  std::this_thread::sleep_for(std::chrono::seconds(3));
  std::cout << "Foo" << std::endl;;
}

void Boo() {
  std::this_thread::sleep_for(std::chrono::seconds(3));
  std::cout << "Boo" << std::endl;;
}

int main() {
  std::cout << "Starting the first thread..." << std::endl;
  std::thread thread1(Foo);

  std::cout << "Starting the second thread..." << std::endl;
  std::thread thread2(Boo);

  std::cout << "Waiting for the threads to finish..." << std::endl;


  //std::this_thread::sleep_for(std::chrono::seconds(5));
  thread1.detach();
  thread2.detach();
  
  std::cout << "Done" << std::endl;
  
  return 0;
}
