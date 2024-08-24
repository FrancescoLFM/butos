#ifndef PROC_H
#define PROC_H

#include <fs/elf.h>

#define DEFAULT_STACK   1024
#define DEFAULT_HEAP    1000000
struct process {
    uintptr_t vaddr;
    elf_t *executable;
    u32 stack_size;
    u32 heap_size;
};

int process_init();
int process_exec(elf_t *elf);

#endif