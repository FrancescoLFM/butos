#include <include/syscalls.h>
#include "ulib.h"

/* gcc can emit calls to memcpy/memset even in freestanding code */
void *memcpy(void *dest, const void *src, size_t n)
{
    uint8_t *dest_ = dest;
    const uint8_t *src_ = src;

    while (n-- > 0)
        *dest_++ = *src_++;

    return dest;
}

void *memset(void *s, int c, size_t n)
{
    uint8_t *s_ = s;

    while (n-- > 0)
        *s_++ = (uint8_t) c;

    return s;
}

size_t strlen(const char *s)
{
    size_t len = 0;

    while (s[len])
        len++;

    return len;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s1 == *s2)
        s1++, s2++;

    return (unsigned char) *s1 - (unsigned char) *s2;
}

char *strchr(const char *s, int c)
{
    do {
        if (*s == (char) c)
            return (char *) s;
    } while (*s++);

    return NULL;
}

char *strdup(const char *s)
{
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);

    if (copy != NULL)
        memcpy(copy, s, len);

    return copy;
}

static int hexval(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

int atoi(const char *num)
{
    long value = 0;
    int neg = 0;

    if (num[0] == '0' && num[1] == 'x') {
        for (num += 2; hexval(*num) >= 0; num++)
            value = value * 16 + hexval(*num);
    } else {
        if (*num == '-')
            neg = 1, num++;
        for (; *num >= '0' && *num <= '9'; num++)
            value = value * 10 + *num - '0';
    }

    return (int) (neg ? -value : value);
}

void *malloc(size_t size)
{
    void *ptr = NULL;

    kalloc_syscall(&ptr, size);

    return ptr;
}

void free(void *ptr)
{
    if (ptr != NULL)
        kfree_syscall(ptr);
}
