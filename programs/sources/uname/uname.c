#include <include/syscalls.h>
#include <libs/string.h>

#define UNAME_SYSNAME   BIT(0)
#define UNAME_NODENAME  BIT(1)
#define UNAME_RELEASE   BIT(2)
#define UNAME_VERSION   BIT(3)
#define UNAME_MACHINE   BIT(4)
#define UNAME_ALL       (BIT(5) - 1)

static void usage(void)
{
    puts_syscall("Usage: uname [-asnrvm]\n"
                 "  -a  all the fields below\n"
                 "  -s  kernel name (default)\n"
                 "  -n  node name\n"
                 "  -r  kernel release\n"
                 "  -v  kernel build\n"
                 "  -m  machine\n");
}

int main(int argc, char **argv)
{
    struct utsname name;
    unsigned int fields = 0;
    char *values[5];
    int first = 1;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-' || argv[i][1] == '\0') {
            usage();
            return EXIT_FAILURE;
        }
        for (char *opt = argv[i] + 1; *opt; opt++) {
            switch (*opt) {
                case 'a': fields |= UNAME_ALL; break;
                case 's': fields |= UNAME_SYSNAME; break;
                case 'n': fields |= UNAME_NODENAME; break;
                case 'r': fields |= UNAME_RELEASE; break;
                case 'v': fields |= UNAME_VERSION; break;
                case 'm': fields |= UNAME_MACHINE; break;
                default:
                    usage();
                    return EXIT_FAILURE;
            }
        }
    }
    if (fields == 0)
        fields = UNAME_SYSNAME;

    if (uname_syscall(&name) != FILE_OK) {
        puts_syscall("uname: can't read the system name\n");
        return EXIT_FAILURE;
    }

    values[0] = name.sysname;
    values[1] = name.nodename;
    values[2] = name.release;
    values[3] = name.version;
    values[4] = name.machine;
    for (size_t i = 0; i < ARR_SIZE(values); i++) {
        if (!(fields & BIT(i)))
            continue;
        if (!first)
            putc_syscall(' ');
        puts_syscall(values[i]);
        first = 0;
    }
    putc_syscall('\n');

    return EXIT_SUCCESS;
}
