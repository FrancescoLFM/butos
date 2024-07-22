#include <cpu/proc.h>
#include <include/def.h>
#include <libs/alloc.h>
#include <libs/print.h>

static struct process *current_process;

struct process *process_alloc() {
    struct process *proc;
    proc = kcalloc(1, sizeof(*proc));
    if (proc == NULL)
        puts("Failed to allocate root process\n");
    return proc;
}

void process_dealloc(struct process *proc) {
    kfree(proc);
}

int proot_init(int (*root_subroutine)()) {
    current_process = process_alloc();
    if (current_process == NULL)
        return EXIT_FAILURE;
    current_process->subroutine = root_subroutine;
    root_subroutine();

    process_dealloc(current_process);
    return 0;
}
