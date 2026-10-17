/*
    loadkeys - select the keyboard layout
    The layouts are built into the kernel (src/drivers/keymaps.h).
*/
#include <include/syscalls.h>

#define LAYOUT_SIZE     16

static void usage(void)
{
    puts_syscall("Usage: loadkeys [-q] [layout]\n"
                 "  layout  it, us (without it prints the current one)\n"
                 "  -q      don't print the selected layout\n");
}

int main(int argc, char **argv)
{
    char layout[LAYOUT_SIZE];
    char *name = NULL;
    int quiet = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-' && argv[i][1] == 'q' && argv[i][2] == '\0') {
            quiet = 1;
        } else if (argv[i][0] != '-' && name == NULL) {
            name = argv[i];
        } else {
            usage();
            return EXIT_FAILURE;
        }
    }

    switch (keymap_syscall(name, layout, sizeof(layout))) {
    case FILE_OK:
        break;
    case FILE_NOT_FOUND:
        printf_syscall("loadkeys: %s: unknown layout (it, us)\n", name);
        return EXIT_FAILURE;
    default:
        puts_syscall("loadkeys: can't read the layout\n");
        return EXIT_FAILURE;
    }
    if (!quiet || name == NULL)
        printf_syscall("%s\n", layout);

    return EXIT_SUCCESS;
}
