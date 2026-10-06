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

typedef enum {
    PROC_OK = 0,
    PROC_NOT_FOUND,
    PROC_INVALID_EXEC,
    PROC_EXEC_FAILED,
} proc_status_t;

int process_init(fat_fs_t *root_fs);
int process_exec(elf_t *elf, int *exit_code);
/* Loads the executable at path from the root filesystem and runs it to completion */
proc_status_t process_spawn(char *path, int *exit_code);

#endif