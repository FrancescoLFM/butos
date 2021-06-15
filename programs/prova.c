#include "../clibs/string.h"
#include <vga.h>
#include <asm.h>

void puts(char* string);

char* htos(char* buffer, uint32_t hex, uint8_t size);

void main();

void _start()
{
    main();
    stop();
}

void main()
{
    char numero[9];
    uint8_t len = strlen("Francesco");
    puts(htos(numero, len, sizeof(len)));
}

void puts(char* string)
{
    print_pm(WHITE_ON_BLACK, string);
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