#include <include/syscalls.h>
#include <libs/string.h>

#define RM_PATH_MAX     256

/* Options */
static int recursive;   /* -r: remove directories and their content */
static int empty_dirs;  /* -d: remove empty directories */
static int force;       /* -f: ignore missing files */

static void err(const char *path, const char *msg)
{
    char *parts[] = {"rm: ", (char *) path, ": ", (char *) msg, "\n"};

    for (size_t i = 0; i < ARR_SIZE(parts); i++)
        write_syscall(STDERR_FILENO, parts[i], strlen(parts[i]));
}

static const char *strerror(int status)
{
    switch (status) {
        case FILE_NOT_FOUND:    return "no such file or directory";
        case FILE_INVALID:      return "invalid path";
        case FILE_BUSY:         return "in use";
        case FILE_NOT_EMPTY:    return "directory not empty";
        case FILE_IO_ERROR:     return "I/O error";
        default:                return "error";
    }
}

/* Directories are the paths readdir accepts, even when empty */
static int is_dir(char *path)
{
    struct dirent entry;

    return readdir_syscall(path, 0, &entry) >= 0;
}

static int rm(char *path);

/* Removes everything inside the directory path, entries that fail are skipped */
static int rm_content(char *path)
{
    char child[RM_PATH_MAX];
    struct dirent entry;
    size_t len = strlen(path);
    int index = 0;
    int ret = EXIT_SUCCESS;

    /* Removing an entry shifts the next ones back to its index */
    while (readdir_syscall(path, index, &entry) > 0) {
        if (len + 1 + strlen(entry.name) >= sizeof(child)) {
            err(path, "path too long");
            ret = EXIT_FAILURE;
            index++;
            continue;
        }
        strcpy(child, path);
        if (len == 0 || path[len - 1] != '/')
            strcat(child, "/");
        strcat(child, entry.name);
        if (rm(child)) {
            ret = EXIT_FAILURE;
            index++;
        }
    }

    return ret;
}

static int rm(char *path)
{
    int status;

    if (is_dir(path)) {
        if (!recursive && !empty_dirs) {
            err(path, "is a directory");
            return EXIT_FAILURE;
        }
        if (recursive && rm_content(path))
            return EXIT_FAILURE;
        status = rmdir_syscall(path);
    } else {
        status = remove_syscall(path);
    }

    if (status == FILE_OK || (force && status == FILE_NOT_FOUND))
        return EXIT_SUCCESS;
    err(path, strerror(status));

    return EXIT_FAILURE;
}

static void usage(void)
{
    puts_syscall("Usage: rm [-rdf] path...\n"
                 "  -r  remove directories and their content\n"
                 "  -d  remove empty directories\n"
                 "  -f  ignore missing files\n");
}

int main(int argc, char **argv)
{
    int ret = EXIT_SUCCESS;
    int paths = 0;
    int i;

    /* Options come before the paths */
    for (i = 1; i < argc && argv[i][0] == '-' && argv[i][1] != '\0'; i++) {
        for (char *opt = argv[i] + 1; *opt; opt++) {
            switch (*opt) {
                case 'r':
                case 'R':
                    recursive = 1;
                    break;
                case 'd':
                    empty_dirs = 1;
                    break;
                case 'f':
                    force = 1;
                    break;
                default:
                    usage();
                    return EXIT_FAILURE;
            }
        }
    }

    for (; i < argc; i++, paths++)
        if (rm(argv[i]))
            ret = EXIT_FAILURE;

    if (paths == 0 && !force) {
        usage();
        return EXIT_FAILURE;
    }

    return ret;
}
