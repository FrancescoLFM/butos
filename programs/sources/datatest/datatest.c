#include <include/syscalls.h>

/* Exercises .data, .bss and .rodata loading */
static int counter = 7;
static char bss_buffer[8192];
static const char greeting[] = "datatest: .rodata ok\n";

int main()
{
    int zeroed = 1;

    for (unsigned int i = 0; i < sizeof(bss_buffer); i++)
        if (bss_buffer[i])
            zeroed = 0;

    puts_syscall((char *) greeting);
    printf_syscall("datatest: .data counter = %d (expected 7)\n", counter);
    printf_syscall("datatest: .bss zeroed = %d (expected 1)\n", zeroed);

    counter += 35;
    bss_buffer[100] = 'x';

    return counter;
}
