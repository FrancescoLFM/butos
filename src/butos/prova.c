#include <include/string.h>
#include <include/print.h>
#include <include/asm.h>
#include <include/idt.h>
#include <include/kb.h>

int puts(char* string);

char* htos(char* buffer, uint32_t hex, uint8_t size);

void _start()
{
    isr_install();
    init_kb();
    stop();
}
