#pragma once

#include <iostream>

class SimpleClass {
 public:
  SimpleClass();
  virtual ~SimpleClass() {
    std::cout << "in virtual destructor\n";
  }
  double foo(int y) {
    std::cout << "in simple method\n";
    return y + 1.0;
  }
  virtual int foo() {
    std::cout << "in virtual method\n";
    x = 42;
    return x;
  }

private:
  int x;
};


