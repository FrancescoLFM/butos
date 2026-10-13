#include <include/syscalls.h>

int main(int argc, char **argv) {
  if (argc < 2) {
    puts_syscall("No file specified. Exiting.");
    return -1;
  }

  return create_syscall(argv[1]);
}