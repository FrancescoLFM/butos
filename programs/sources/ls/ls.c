#include <include/syscalls.h>

int main(int argc, char **argv)
{
    const char *path = argc > 1 ? argv[1] : "/";
    struct dirent entry;
    int i = 0, n;

    while ((n = readdir_syscall((char *) path, i, &entry)) > 0) {
        puts_syscall(entry.name);
        if (entry.is_dir)
            putc_syscall('/');
        putc_syscall('\n');
        i++;
    }

    if (n < 0)
        puts_syscall("ls: error\n");

    return 0;
}
