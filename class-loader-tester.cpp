#include <iostream>

#include "./some/package/module.h"
#include "interfaces.h"

static ClassLoader* Loader = nullptr;

using namespace some::package;

int testSimpleClass() {
  Class<SimpleClass>* c =
    reinterpret_cast<Class<SimpleClass>*>(Loader->loadClass("some::package::SimpleClass"));
  if (c) {
    SimpleClass* instance = c->newInstance();
    instance->foo();
    delete instance;
    instance = c->newInstance();
    instance->foo();
    return 0;
  } else {
    return 1;
  }
}

int main(int argc, char** argv) {
  Loader = new ClassLoader();
  int status = testSimpleClass();
  delete Loader;
  return status;
}
