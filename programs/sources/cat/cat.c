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
  int n;

  while ((n = read_syscall(fd, buffer, sizeof(buffer))) > 0) {
    for (int i = 0; i < n; i++)
      putc_syscall(buffer[i]);
  }

  close_syscall(fd);
  return 0;
}