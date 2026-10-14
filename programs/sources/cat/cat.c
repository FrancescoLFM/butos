#include <include/syscalls.h>

static void error(char *msg)
{
  char *end = msg;

  while (*end)
    end++;
  write_syscall(STDERR_FILENO, msg, end - msg);
}

int main(int argc, char **argv) {
  char buffer[512];
  int n;

  if (argc < 2) {
    while ((n = read_syscall(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
      write_syscall(STDOUT_FILENO, buffer, n);
    }
    
    return 0;
  }

  int fd = open_syscall(argv[1]);
  if (fd < 0) {
    error("cat: can't open the file\n");
    return 1;
  }

  while ((n = read_syscall(fd, buffer, sizeof(buffer))) > 0)
    write_syscall(STDOUT_FILENO, buffer, n);

  close_syscall(fd);
  return 0;
}
