#include <libs/string.h>
#include <stdint.h>
#include <libs/alloc.h>

#define CAST(PTR)    ((uint8_t *)(PTR))

/* memory functions */

void *memcpy(void *restrict dest, const void *restrict src, size_t n)
{
    uint8_t *dest_ = dest;
    const uint8_t *src_ = src;

    while (n-->0)
        *dest_++ = *src_++;

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const uint8_t *s1_ = s1;
    const uint8_t *s2_ = s2;

    while (n-->0 && *s1_ == *s2_) {
        s1_++;
        s2_++;
    }
    return *s1_ - *s2_;
}

void *memchr(const void *s, int c, size_t n)
{    
    size_t i = 0;
    const uint8_t *s_ = s;

    n--;
    while (i < n && s_[i] != c)
        i++;

    return s_[i] == c ? (uint8_t *)(s_ + i) : NULL;
}

void *memrchr(const void *s, int c, size_t n)
{
    const uint8_t *s_ = s;

    while (n-->0 && s_[n] != c);

    return s_[n] == c ? (uint8_t *)(s_ + n) : NULL;
}

void *rawmemchr(const void *s, int c)
{
    const uint8_t *s_ = s;
    while (*s_ != c)
        s_++;

    return (uint8_t *)s_;
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
	for (;; ++s) {
		if (*s == c)
			return ((char *)s);
		if (*s == '\0')
			return (NULL);
	}
	/* NOTREACHED */
}

char *strrchr(const char *s, int c)
{
    const char *found, *p;

    c = (unsigned char) c;

    /* Since strchr is fast, we use it rather than the obvious loop.  */

    if (c == '\0')
        return strchr (s, '\0');

    found = NULL;
    while ((p = strchr (s, c)) != NULL) {
      found = p;
      s = p + 1;
    }

  return (char *) found;
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

void tokenize(
        char *restrict tokenized,
        char *restrict str,
        const char *restrict delim
)
{
    int dont_repeat = 0;

    for (; *str; str++) {
        if (strchr(delim, *str) == NULL) {
            *tokenized++ = *str;
            dont_repeat = 0;
        } else if (dont_repeat == 0) {
            *tokenized++ = '\0';
            dont_repeat = 1;
        }
    }

    *tokenized++ = '\0';
    *tokenized  = '\0';

}

/**
 * NOTA: La tokenizzazione avviene tutta alla prima chiamata,
 * diversamente dalla funzione standard. Nelle chiamate
 * successive delim può anche essere messo a NULL perché tanto
 * non ha effetto. In seguito magari cercherò di farla
 * comportare nel modo standard, ma per ora va bene così
 */
char *strtok(char *restrict str, const char *restrict delim)
{
    static char *tokenized = NULL;
    static char *p = NULL;

    if (str) {
        if (tokenized != NULL)
            kfree(tokenized);
        p = tokenized = kalloc(strlen(str) + 2);
        if (tokenized == NULL)
            return NULL;
        else
            tokenize(tokenized, str, delim);
    } else if (p) {
        while (*p++);
        if (*p == '\0')
            p = NULL;
    }

    return p;
}

size_t strcspn(const char *s1, register const char *s2)
{
	register const char *p, *spanp;
	register char c, sc;

	/*
	 * Stop as soon as we find any character from s2.  Note that there
	 * must be a NUL in s2; it suffices to stop when we find that, too.
	 */
	for (p = s1;;) {
		c = *p++;
		spanp = s2;
		do {
			if ((sc = *spanp++) == c)
				return (p - 1 - s1);
		} while (sc != 0);
	}
	/* NOTREACHED */
}

char *strsep(char **stringp, const char *delim) 
{
    char *begin, *end;
    begin = *stringp;
    if (begin == NULL)
        return NULL;
    /* Find the end of the token.  */
    end = begin + strcspn(begin, delim);
    if (*end) {
        /* Terminate the token and set *STRINGP past NUL character.  */
        *end++ = '\0';
        *stringp = end;
    } else
        /* No more delimiters; this is the last token.  */
        *stringp = NULL;
    return begin;
}


char *strdup(const char *restrict s) {
    size_t len;
	char *copy;

	len = strlen(s) + 1;
	if (!(copy = (char *) kcalloc(len, sizeof(char))))
		return (NULL);
	memcpy(copy, s, len);
	return (copy);
}

