#include <include/syscalls.h>
#include <libs/string.h>

#define BUFFER_SIZE         256
#define PATH_SIZE           128
#define SHARED_BUFFER_SIZE  512

#define UNRECOGNIZED_ERROR -1
#define QUIT_REQUEST        42

#define EOF -1

typedef struct shared_buffer {
  char *buffer;
  size_t size;
  int cursor; // The last index with valid data, the rest could be likely garbage
} shared_buffer_t;

typedef struct file_status {
  char *path;
  int filename_index;
  int is_changed;
  int is_saved;
  int fd; // -1 if file doesn't exists (yet)
} file_status_t;

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
  unsigned char c;

  if (read_syscall(fd, &c, 1) <= 0)
    return EOF;

  return c;
}

int getchar() {
  return getc(STDIN_FILENO);
}

int get_filename_index(char *path) {
  char *filename, *temp;

  filename = path;
  temp = path;
  while ((temp = strchr(temp, '/'))) {
    temp++;
    filename = temp;
  }

  return (int) (filename - path);
}

int insert_mode(shared_buffer_t *shared_buffer, file_status_t *file_status) {
  size_t n = 0;
  int c;

  puts("INSERT MODE\n==============");
  while ((c = getchar()) != EOF) {
    if (n >= shared_buffer->size) {
      size_t new_size = shared_buffer->size * 2;
      shared_buffer->buffer = krealloc_syscall((void **) &(shared_buffer->buffer), new_size);
      if (shared_buffer->buffer == NULL) {
        return -2;
      }
    }
    shared_buffer->buffer[n] = c;
    shared_buffer->cursor = n;
    n++;
  }
  puts("\n==============");

  file_status->is_changed = 1;
  file_status->is_saved = 0;
  return 0;
}

void print_file(shared_buffer_t *shared_buffer) {
  for (int i = 0; i <= shared_buffer->cursor; i++) {
    putchar(shared_buffer->buffer[i]);
  }
  putchar('\n');
}

void print_help(void) {
  puts("Commands:");
  puts("  i  insert mode (Ctrl+D to leave)");
  puts("  a  append mode (CTRL+D to leave)");
  puts("  p  print buffer");
  puts("  w  write buffer to file");
  puts("  q  quit");
  puts("  h  show this help");
}

int write_file(shared_buffer_t *shared_buffer, file_status_t *file_status) {
  if (file_status->fd < 0) {
    if (create_syscall(file_status->path)) {
      error("Error creating file, check filesystem.\n");
      return -2;
    }
  }

  file_status->fd = open_syscall(file_status->path);
  if (file_status->fd < 0) {
    error("Error opening file, check filesystem.\n");
    return -2;
  }

  if (write_syscall(file_status->fd, shared_buffer->buffer, shared_buffer->cursor + 1) < 0) {
    error("Error writing file, check filesystem.\n");
    return -2;
  }

  file_status->is_saved = 1;
  return 0;
}

int append_mode(shared_buffer_t *shared_buffer, file_status_t *file_status) {
  size_t n = shared_buffer->cursor + 1;
  int c;

  puts("APPEND MODE\n==============");
  print_file(shared_buffer);
  while ((c = getchar()) != EOF) {
    if (n >= shared_buffer->size) {
      size_t new_size = shared_buffer->size * 2;
      shared_buffer->buffer = krealloc_syscall((void **) &(shared_buffer->buffer), new_size);
      if (shared_buffer->buffer == NULL) {
        return -2;
      }
    }
    shared_buffer->buffer[n] = c;
    shared_buffer->cursor = n;
    n++;
  }
  puts("\n==============");

  file_status->is_changed = 1;
  file_status->is_saved = 0;
  return 0;
}

int parse_commands(char *buffer, int size, shared_buffer_t *shared_buffer, file_status_t *file_status) {
  int errno = 0;

  if (size > 1) {
    return UNRECOGNIZED_ERROR;
  }
  if (size < 1) {
    return 0;
  }

  switch (*buffer) {
  case 'i':
    errno = insert_mode(shared_buffer, file_status);
  break;
  case 'p':
    print_file(shared_buffer);
  break;
  case 'w':
    errno = write_file(shared_buffer, file_status);
  break;
  case 'h':
    print_help();
  break;
  case 'a':
    append_mode(shared_buffer, file_status);
  break;
  case 'q':
    return QUIT_REQUEST;
  
  default:
    return UNRECOGNIZED_ERROR;
  }
  
  return errno;
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
  char path[PATH_SIZE];
  int i, n, c;
  int errno;
  file_status_t file_status;

  file_status.path = path;

  if (argc < 2) {
    puts("No file provided, fallbacking to a.txt");
    strcpy(path, "/a.txt");
    file_status.filename_index = 1;
  } else {
    if (strlen(argv[1]) > PATH_SIZE) {
      error ("BUFFER ERROR: Path too long.");
      return -1;
    }
    strcpy(file_status.path, argv[1]);
    file_status.filename_index = get_filename_index(file_status.path);
  }

  if (alloc_shared_buffer(&shared_buffer)) {
    error("FATAL ERROR: No memory available.");
    return -1;
  }

  file_status.fd = open_syscall(file_status.path);
  if (file_status.fd > 0) {
    while ((n = read_syscall(file_status.fd, shared_buffer.buffer, shared_buffer.size)) > 0) {
      if ((size_t) n == shared_buffer.size) {
        shared_buffer.buffer = krealloc_syscall(shared_buffer.buffer, shared_buffer.size * 2);
        if (shared_buffer.buffer == NULL) {
          error("FATAL ERROR: File is too big, no memory available.");
          return -1;
        }
      }
    };
  }

  file_status.is_changed = 0;
  file_status.is_saved = 1;

  for (;;) {
    prompt_input(&file_status.path[file_status.filename_index]);

    i = 0;
    while((c = getchar()) != '\n' && c != EOF) {
      buffer[i++] = c;
      if (i >= BUFFER_SIZE) {
        break;
      }
    }

    if ((errno = parse_commands(buffer, i, &shared_buffer, &file_status))) {
      if (errno == QUIT_REQUEST) {
        if (file_status.is_changed && !file_status.is_saved) {
          puts_ns("Quit without saving changes? [s/N]: ");
          int choice = getchar();
          if (choice != 's') {
            continue;
          }
        }
        
        goto exit;
      }
      if (errno == UNRECOGNIZED_ERROR) {
        error("Command not recognized. Write h for full list of commands.\n");
      }
    }
  }

exit:
  kfree_syscall(shared_buffer.buffer);
  return 0;
}