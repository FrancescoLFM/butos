#include <include/string.h>
#include <include/print.h>
#include <include/asm.h>

int puts(char* string);

char* htos(char* buffer, uint32_t hex, uint8_t size);

void _start()
{
    char buffer[sizeof(int)*2 + 1];

    puts(htos(buffer, puts("byte: 0x"), sizeof(int)));
    io_delay();
    stop();
}

int puts(char* string)
{
    return print_pm(COLOR(WHITE, BLACK), string);
}

char* htos(char* buffer, uint32_t hex, uint8_t size)
{
    char* digits = "0123456789abcdef";
    uint8_t nibbles = size << 1;

    for (int i = 0; i < nibbles; i++) {
        buffer[nibbles - (i + 1)] = digits[hex & 0x0F];
        hex >>= 4;
    }

    buffer[nibbles] = 0;

    return buffer;
}