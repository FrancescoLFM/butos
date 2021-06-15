#include "../clibs/string.h"
#include <vga.h>
#include <asm.h>

void puts(char* string);

char* htos(char* buffer, uint32_t hex, uint8_t size);

uint8_t test_a20();

void _start()
{
    uint8_t test = test_a20();
    puts((test) ? "a20 line enabled" : "a20 line disabled");
    stop();
}

void puts(char* string)
{
    print_pm(WHITE_ON_BLACK, string);
}

uint8_t test_a20()
{
    uint8_t temp, ret;
    uint8_t *mem1 = (uint8_t*)(0x112345);
    uint8_t *mem2 = (uint8_t*)(0x012345);

    temp = *mem1;

    *mem1 = ~(*mem2);
    ret = (*mem1 != *mem2);

    *mem1 = temp;
    return ret;
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