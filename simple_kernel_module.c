#include <linux/module.h>
#include <linux/kernel.h>

int
init_module() {
  printk("Hello from newly created module!\n");
  return 0;
}

void
cleanup_module() {
  printk("Unloading newly created module\n");
}

MODULE_LICENSE("GPL");
