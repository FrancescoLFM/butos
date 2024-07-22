#include <include/def.h>

#ifndef STRING_H
#define STRING_H

void *memcpy(void *restrict dest, const void *restrict src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memchr(const void *s, int c, size_t n);
void *memrchr(const void *s, int c, size_t n);
void *rawmemchr(const void *s, int c);
void *memset(void *s, int c, size_t n);

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strrchr(const char *s, int c);
size_t strlen(const char *s);
size_t strnlen(const char *s, size_t maxlen);
char *strcat(char *restrict dest, const char *restrict src);
char *strncat(char *restrict dest, const char *restrict src, size_t n);
char *strcpy(char *restrict dest, const char *restrict src);
char *strncpy(char *restrict dest, const char *restrict src, size_t n);
char *strtok(char *restrict str, const char *restrict delim);
char *strdup(const char *restrict s);
size_t strcspn(const char *s1, register const char *s2);
char *strsep(char **stringp, const char *delim);

#endif
