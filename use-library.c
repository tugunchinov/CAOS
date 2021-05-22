#include <stdio.h>
#include <dlfcn.h>

typedef void (*void_func_t)();
typedef int (*int_func_t)();

int
main(int argc, char** argv) {
  const char* lib_name = argv[1];
  void* lib = dlopen(lib_name, RTLD_NOW);
  if (lib == NULL) {
    fprintf(stderr, "Can't load library: %s\n", dlerror());
    return 1;
  }

  void_func_t lib__do_something = dlsym(lib, "do_something");
  int_func_t lib__main = dlsym(lib, "main");

  lib__do_something();
  return lib__main();
}
