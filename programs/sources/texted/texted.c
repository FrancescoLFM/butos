#include <include/syscalls.h>
#include <libs/string.h>

#define BUFFER_SIZE         256
#define SHARED_BUFFER_SIZE  512

#define UNRECOGNIZED_ERROR -1
#define QUIT_REQUEST        42

#define EOF -1

typedef struct shared_buffer {
  char *buffer;
  size_t size;
  int cursor; // The last index with valid data, the rest could be likely garbage
} shared_buffer_t;

void putc(int fd, char c) {
  char character = c;

  write_syscall(fd, &character, 1);
}

void putchar(char c) {
  char character = c;

  write_syscall(STDOUT_FILENO, &character, 1);
}

void puts_ns(char *str) {
  for (int i = 0; str[i]; i++) {
    putchar(str[i]);
  }
}

void puts(char *str) {
  for (int i = 0; str[i]; i++) {
    putchar(str[i]);
  }

  putchar('\n');
}

static void error(char *msg)
{
  char *end = msg;

  while (*end)
    end++;
  write_syscall(STDERR_FILENO, msg, end - msg);
}

void prompt_input(char *current_file) {
  puts_ns(current_file);
  puts_ns(" > ");
}

int getc(int fd) {
  int c = -1;

  read_syscall(fd, &c, 1);

  return c;
}

int getchar() {
  int c = -1;

  read_syscall(STDIN_FILENO, &c, 1);

  return c;
}

void insert_mode(shared_buffer_t *shared_buffer) {
  size_t n = 0;
  int c;

  puts("INSERT MODE\n==============");
  while ((c = getchar()) != EOF) {
    if (n >= shared_buffer->size) {
      size_t new_size = shared_buffer->size * 2;
      krealloc_syscall((void **) &(shared_buffer->buffer), new_size);
    }
    shared_buffer->buffer[n] = c;
    shared_buffer->cursor = n;
    n++;
  }
  puts("\n==============");
}

void print_file(shared_buffer_t *shared_buffer) {
  for (int i = 0; i <= shared_buffer->cursor; i++) {
    putchar(shared_buffer->buffer[i]);
  }
  putchar('\n');
}

int parse_commands(char *buffer, int size, shared_buffer_t *shared_buffer) {
  if (size > 1) {
    return UNRECOGNIZED_ERROR;
  }
  if (size < 1) {
    return 0;
  }

  switch (*buffer) {
  case 'i':
    insert_mode(shared_buffer);
  break;
  case 'p':
    print_file(shared_buffer);
  break;
  case 'q':
    return QUIT_REQUEST;
  break;
  
  default:
    return UNRECOGNIZED_ERROR;
  }
  
  return 0;
}

int alloc_shared_buffer(shared_buffer_t *shared_buffer) {
  kalloc_syscall((void **) &(shared_buffer->buffer), SHARED_BUFFER_SIZE);
  if (shared_buffer->buffer == NULL) {
    return -1;
  }

  shared_buffer->size = SHARED_BUFFER_SIZE;
  shared_buffer->cursor = -1;

  return 0;
}

int main(int argc, char **argv) {
  char buffer[BUFFER_SIZE];
  shared_buffer_t shared_buffer;
  char filename[100];
  int i;
  char c;
  int errno;

  if (argc < 2) {
    puts("No file provided, fallbacking to a.txt");
    strcpy(filename, "a.txt");
  } else {
    strcpy(filename, argv[1]);
  }

  if (alloc_shared_buffer(&shared_buffer)) {
    error ("FATAL ERROR: No memory available.");
    return -1;
  }

  for (;;) {
    prompt_input(filename);

    i = 0;
    while((c = getchar()) != '\n') {
      buffer[i++] = c;
      if (i >= BUFFER_SIZE) {
        break;
      }
    }

    if ((errno = parse_commands(buffer, i, &shared_buffer))) {
      if (errno == QUIT_REQUEST) {
        goto exit;
      }
      error("Command not recognized. Write h for full list of commands.");
      putc(STDERR_FILENO, '\n');
    }
  }

exit:
  kfree_syscall(shared_buffer.buffer);
  return 0;
}