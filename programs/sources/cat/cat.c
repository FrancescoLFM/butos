#include <include/syscalls.h>

static void error(char *msg)
{
  char *end = msg;

  while (*end)
    end++;
  write_syscall(STDERR_FILENO, msg, end - msg);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    error("Error: No file specified. Exiting.\n");
    return 1;
  }

  int fd = open_syscall(argv[1]);
  if (fd < 0) {
    error("cat: can't open the file\n");
    return 1;
  }
  char buffer[512];
  int n;

  while ((n = read_syscall(fd, buffer, sizeof(buffer))) > 0)
    write_syscall(STDOUT_FILENO, buffer, n);

  close_syscall(fd);
  return 0;
}
