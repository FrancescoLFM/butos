#include <include/syscalls.h>
#include "ulib.h"

void *malloc(size_t size)
{
    void *ptr = NULL;

    kalloc_syscall(&ptr, size);

    return ptr;
}

void *realloc(void *ptr, size_t size)
{
    return krealloc_syscall(ptr, size);
}

void free(void *ptr)
{
    if (ptr != NULL)
        kfree_syscall(ptr);
}
