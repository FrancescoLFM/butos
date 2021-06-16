#include "../clibs/string.h"
#include <print.h>
#include <asm.h>

void puts(char* string);

char* htos(char* buffer, uint32_t hex, uint8_t size);

uint8_t test_a20();

void _start()
{
    puts("stringa");
    stop();
}

void puts(char* string)
{
    print_pm(WHITE_ON_BLACK, string);
}

uint8_t test_a20()
{
    uint8_t ret;
    uint8_t mem1 = 0x00;
    uint8_t *mem2 = &mem1;

    mem2 = (uint8_t*)((uint32_t)(&mem1) ^ 0x100000);

    mem1 = ~(*mem2);
    ret = (mem1 != *mem2);

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