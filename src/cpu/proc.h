#ifndef PROC_H
#define PROC_H

#include <fs/elf.h>

#define DEFAULT_STACK   1024
#define DEFAULT_HEAP    1000000
/* Limits on the arguments passed to process_spawn */
#define PROC_MAX_ARGS       32
#define PROC_MAX_ARGS_SIZE  1024
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
    PROC_INVALID_ARGS,
} proc_status_t;

int process_init(fat_fs_t *root_fs);
/* Runs the loaded elf as main(argc, argv), argv must be in kernel memory */
int process_exec(elf_t *elf, int argc, char **argv, int *exit_code);
/* Loads the executable at path from the root filesystem and runs it to completion,
   argv is NULL terminated with the program name first (NULL means just path) */
proc_status_t process_spawn(char *path, char **argv, int *exit_code);
/* Nesting depth of the running process, 0 when called from the kernel */
int process_depth(void);

#endif