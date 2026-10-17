#include <include/syscalls.h>

#define BUFFER_LEN  512

static void error(char *msg)
{
  char *end = msg;

  while (*end)
    end++;
  write_syscall(STDERR_FILENO, msg, end - msg);
}

int main(int argc, char **argv) {
  char buffer[BUFFER_LEN];
  int n;

  if (argc < 2) {
    while ((n = read_syscall(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
      write_syscall(STDOUT_FILENO, buffer, n);
    }
    
    return 0;
  }

  for (int i = 1; i < argc; i++) {
    int fd = open_syscall(argv[i]);
    if (fd < 0) {
      error("cat: cannot open the file ");
      error(argv[i]);
      error("\n");
      return 1;
    }

    while ((n = read_syscall(fd, buffer, sizeof(buffer))) > 0)
      write_syscall(STDOUT_FILENO, buffer, n);

    close_syscall(fd);
  }

  write_syscall(STDOUT_FILENO, "\n", 1);

  return 0;
}
