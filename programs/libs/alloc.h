#ifndef ULIBS_ALLOC_H
#define ULIBS_ALLOC_H

#include <include/def.h>
#include <include/syscalls.h>

/* User-side kalloc/krealloc/kfree on the heap syscalls (used by libs/string.c) */

static inline void *kalloc(size_t size)
{
    void *ptr = NULL;

    kalloc_syscall(&ptr, size);

    return ptr;
}

static inline void *kcalloc(size_t nmemb, size_t size)
{
    uint8_t *ptr = kalloc(nmemb * size);

    if (ptr != NULL)
        for (size_t i = 0; i < nmemb * size; i++)
            ptr[i] = 0;

    return ptr;
}

static inline void *krealloc(void *ptr, size_t size)
{
    return krealloc_syscall(ptr, size);
}

static inline void kfree(void *ptr)
{
    if (ptr != NULL)
        kfree_syscall(ptr);
}

#endif
