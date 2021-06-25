#include <include/string.h>
#include <include/print.h>
#include <include/asm.h>
#include <include/idt.h>
#include <include/kb.h>

void _start()
{
    isr_install();
    init_kb();
    stop();
}
