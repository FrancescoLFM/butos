#ifndef ULIB_H
#define ULIB_H

#include <include/def.h>

/* Minimal libc subset for butosh, heap memory comes from the kernel via syscalls */

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
char *strchr(const char *s, int c);
char *strdup(const char *s);
int atoi(const char *num);

void *malloc(size_t size);
void free(void *ptr);

#endif
