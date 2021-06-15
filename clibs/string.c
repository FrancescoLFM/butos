#include "string.h"
#include "def.h"

void *memcpy(void *to, const void *from, int n)
{
    uint8_t *cfrom = (uint8_t *)from;
    uint8_t *cto = (uint8_t *)to;

    for(int i = 0; i < n; i++)
        cto[i] = cfrom[i];
    return to;
}

void *memccpy(void *to, const void *from, int c, int n)
{
    uint8_t *cfrom = (uint8_t *)from;
    uint8_t *cto = (uint8_t *)to;
    int i;

    for(i = 0; cfrom[i] != c && i < n; i++)
        cto[i] = cfrom[i];
    if(cto[i-1] == c) return to;
    return NULL;
}

int strcmp(const char *str1, const char *str2) // testato, funziona
{
    int i;
    for(i = 0; str1[i] && str2[i]; i++)
        if(str1[i] != str2[i]) 
            return 1;
    
    return (str1[i] != str2[i]);
}

int strncmp(const char *str1, const char *str2, size_t n) // testato, funziona
{
    for(size_t i = 0; str1[i] && str2[i] && i < n; i++)
        if(str1[i] != str2[i]) 
            return 1;
    return 0;
}

size_t strlen(const char *str) // testato, funziona
{
    size_t len = 0;

    for(; *str++; len++);

    return len;
}

char *strcpy(char *to, const char *from) // testato, funziona
{
    int i;

    for(i = 0; from[i]; i++) to[i] = from[i];
    to[i] = '\0';

    return to;
}

char *strncpy(char *to, const char *from, size_t n) // testato, funziona
{
    size_t i;
    for(i = 0; from[i] && i < n; i++) to[i] = from[i];
    to[i] = '\0';

    return to;
}


