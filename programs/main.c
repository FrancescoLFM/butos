#include "vga.h"
#include "asm.h"

volatile char* cursor = (volatile char*)0xb8000;

char* carriage_return(char volatile* cursor);

void _start()
{
    // enable_cursor(13, 14);
    // inc_cursor(80);
    for (int i = 0; i < 25; i++)
        print_pm(0b00000111, "Lanfredi gay, ma a 32 bit\n\r");
    
    stop();
}