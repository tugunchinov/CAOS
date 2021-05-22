#include <stdio.h>

#include <dlfcn.h>

typedef double (*function_t)(double);

int
main(int argc, char** argv) {
  const char* lib_name = argv[1];
  const char* func_name  = argv[2];

  void* lib = dlopen(lib_name, RTLD_NOW | RTLD_LOCAL);
  if (lib == NULL) {
    fprintf(stderr, "dlopen: %s", dlerror());
    return 1;
  }

  void* func_ptr = dlsym(lib, func_name);
  if (func_ptr == NULL) {
    fprintf(stderr, "dlopen: %s", dlerror());
    return 2;
  }

  function_t func = func_ptr;

  double y = 0.0;
  while (scanf("%lf", &y) > 0) {
    printf("%.3f\n", func(y));
  }

  dlclose(lib);
  
  return 0;
}
