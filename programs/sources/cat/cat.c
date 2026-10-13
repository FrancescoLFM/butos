#include <include/syscalls.h>

int main(int argc, char **argv) {
  if (argc < 2) {
    puts_syscall("Error: No file specified. Exiting.");
    return 1;
  }

  int fd = open_syscall(argv[1]);
  if (fd < 0) {
    return 1;
  }
  char buffer[512];
  int eof = 0;

  while(!eof) {
    eof = read_syscall(fd, buffer, 512);
    for (int i = 0; i < 512; i++) {
      putc_syscall(buffer[i]);
    }
  }


  return 0;
}