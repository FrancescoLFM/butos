#include <include/syscalls.h>
#include <libs/string.h>

#define MKDIR_PATH_MAX  256

static void err(const char *path, const char *msg)
{
    char *parts[] = {"mkdir: ", (char *) path, ": ", (char *) msg, "\n"};

    for (size_t i = 0; i < ARR_SIZE(parts); i++)
        write_syscall(STDERR_FILENO, parts[i], strlen(parts[i]));
}

static const char *strerror(int status)
{
    switch (status) {
        case FILE_NOT_FOUND:    return "no such directory";
        case FILE_INVALID:      return "invalid name (only 8.3 names)";
        case FILE_EXISTS:       return "already exists";
        case FILE_IO_ERROR:     return "I/O error";
        default:                return "error";
    }
}

static int is_dir(char *path)
{
    struct dirent entry;

    return readdir_syscall(path, 0, &entry) >= 0;
}

/* -p: creates every missing directory of path, existing ones are fine */
static int mkdir_parents(char *path)
{
    char prefix[MKDIR_PATH_MAX];
    size_t len = strlen(path);
    int status;

    if (len >= sizeof(prefix)) {
        err(path, "path too long");
        return EXIT_FAILURE;
    }
    strcpy(prefix, path);

    for (size_t i = 1; i <= len; i++) {
        if (prefix[i] != '/' && prefix[i] != '\0')
            continue;
        prefix[i] = '\0';
        status = mkdir_syscall(prefix);
        if (status != FILE_OK && !(status == FILE_EXISTS && is_dir(prefix))) {
            err(prefix, status == FILE_EXISTS ? "not a directory" : strerror(status));
            return EXIT_FAILURE;
        }
        prefix[i] = path[i];
    }

    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    int parents = 0;
    int ret = EXIT_SUCCESS;
    int status;
    int i = 1;

    if (i < argc && strcmp(argv[i], "-p") == 0) {
        parents = 1;
        i++;
    }
    if (i >= argc) {
        puts_syscall("Usage: mkdir [-p] dir...\n"
                     "  -p  create the missing parent directories, no error if it exists\n");
        return EXIT_FAILURE;
    }

    for (; i < argc; i++) {
        if (parents) {
            if (mkdir_parents(argv[i]))
                ret = EXIT_FAILURE;
            continue;
        }
        status = mkdir_syscall(argv[i]);
        if (status != FILE_OK) {
            err(argv[i], strerror(status));
            ret = EXIT_FAILURE;
        }
    }

    return ret;
}
