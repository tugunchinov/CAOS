#include <stdio.h>
#include <setjmp.h>

jmp_buf ebuf;
void
f2(void);

int
main(int argc, char** argv) {
  _Bool first = 1;
  int i = 0;
  printf("1 ");
  i = setjmp(ebuf);
  if (first) {
    first = !first;
    f2();
    printf("This won't be printed");
  }
  printf("%d\n", i);
  return 0;
}

void
f2(void) {
  printf("2 ");
  longjmp(ebuf, 3);
}
