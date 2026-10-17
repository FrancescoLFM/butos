#ifndef ULIB_H
#define ULIB_H

#include <include/def.h>

/* Minimal libc subset for butosh, heap memory comes from the kernel via syscalls.
   The string functions and atoi come from libbutos (libs/string.h, libs/atoi.h) */

void *malloc(size_t size);
/* NULL ptr allocates, size 0 frees; on failure returns NULL and ptr is still valid */
void *realloc(void *ptr, size_t size);
void free(void *ptr);

#endif
