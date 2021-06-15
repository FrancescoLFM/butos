#include <vga.h>
#include <asm.h>

char* htos(char* buffer, uint32_t hex, uint8_t size);

void _start()
{
    char hexstring[9] = {0};
    disable_cursor();
    print_pm(WHITE_ON_BLACK, "esp: 0x");
    
    uint32_t esp;
    asm volatile ("mov %%esp, %%eax"
                 : "=a" (esp));
                

    print_pm(WHITE_ON_BLACK, htos(hexstring, esp, 32));
    
    stop();
}

char* htos(char* buffer, uint32_t hex, uint8_t size)
{
    char* digits = "0123456789abcdef";
    uint8_t nibbles = size >> 2;

    for (int i = 0; i < nibbles; i++) {
        buffer[nibbles - (i + 1)] = digits[hex & 0x0F];
        hex >>= 4;
    }

    buffer[nibbles] = 0;

    return buffer;
}