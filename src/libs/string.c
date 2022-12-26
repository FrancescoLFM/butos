#include <libs/string.h>

#define CAST(PTR)    ((uint8_t *)(PTR))

/* memory functions */

void *memcpy(void *restrict dest, const void *restrict src, size_t n)
{
    void *dest_ = dest;

    while (n-->0)
        *CAST(dest++) = *CAST(src++);

    return dest_;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    while (n-->0 && *CAST(s1) == *CAST(s2)) {
        CAST(s1++);
        CAST(s2++);
    }
    return *(char *)s1 - *(char *)s2;
}

void *memchr(const void *s, int c, size_t n)
{    
    size_t i = 0;

    while (i < n && CAST(s)[i] != c)
        i++;

    return CAST(s)[i] == c ? CAST(s)+i : NULL;
}

void *memrchr(const void *s, int c, size_t n)
{
    while (n-->0 && ((uint8_t *)s)[n] != c);

    return CAST(s)[n] == c ? (CAST(s) + n) : NULL;
}

void *rawmemchr(const void *s, int c)
{
    while (*CAST(s) != c)
        CAST(s++);

    return CAST(s);
}

void *memset(void *s, int c, size_t n)
{
    uint8_t *p;

    p = s;
    while (n-->0)
        *p++ = c;

    return s;
}

/* funzioni per le stringhe */

int strcmp(const char *s1, const char *s2)
{
    while (*s1 == *s2 && *s1 && *s2) {
        s1++;
        s2++;
    }

    return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    while (n-->0 && *s1 == *s2 && *s1 && *s2) {
        s1++;
        s2++;
    }

    return *s1 - *s2;
}

char *strchr(const char *s, int c)
{
    while (*s && *s != c) s++;
    if (*s == c)
        return (char *)s;
    return NULL;
}

char *strrchr(const char *s, int c)
{
    const char *found;

    found = rawmemchr(s, '\0');
    while (found-->s && *found != c);
    if (*found == c)
        return (char *)found;
    return NULL;

}

size_t strlen(const char *s)
{
    return (char *)rawmemchr(s, '\0') - s;
}

size_t strnlen(const char *s, size_t maxlen)
{
    const char *p = s;
    while (maxlen-->0 && *p) p++;

    return p - s;
}

char *strcat(char *restrict dest, const char *restrict src)
{
    char *end;

    end = rawmemchr(dest, '\0');

    while (*src)
        *end++ = *src++;
    *end = '\0';

    return dest;
}

char *strcpy(char *restrict dest, const char *restrict src)
{
    char *start = dest;

    while (*src)
        *start++ = *src++;
    *start = '\0';

    return dest;
}

/* copia al più n byte di src, fra cui il terminatore è contato*/
char *strncpy(char *restrict dest, const char *restrict src, size_t n)
{
    char *start = dest;

    while (*src && n-->0)
        *start++ = *dest++;
    /* oss: se esco per *src = '\0', n non viene aggiornato.
     * se è ancora non consumato ne gioco uno per copiare il
     * terminatore */
    if (n > 0) *start = *dest;

    return dest;
}

/* dest deve avere strlen(dest) + n + 1 byte disponibili */
char *strncat(char *restrict dest, const char *restrict src, size_t n)
{
    char *end;

    end = rawmemchr(dest, '\0');

    while (n-->0 && *src)
        *end++ = *src++;
    *end = '\0';

    return dest;
}
