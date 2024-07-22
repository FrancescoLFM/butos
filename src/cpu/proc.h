typedef unsigned long pid_t;

struct process {
    pid_t id;
    int (*subroutine)();
    struct process *parent;
    struct process *child;
};

int proot_init(int (*root_subroutine)());