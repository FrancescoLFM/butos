#include <include/def.h>

#ifndef STRING_H
#define STRING_H

void *memcpy(void *to, const void *from, size_t n);

void *memccpy(void *to, const void *from, size_t c, size_t n);

/*
    return 1 if the strings aren't equal
    return 0 if the strings are equal
*/
int strcmp(const char *from, const char *to); // compare two strings (like cmp instruction)

/*
    in n characters 
    return 1 if the strings aren't equal
    return 0 if the strings are equal
*/
int strncmp(const char *str1, const char *str2, size_t n);

/* 
    null character is excluded
*/
size_t strlen(const char *str); // return the length of a string

/*
    returns the pointer "to"
*/
char *strcpy(char *to, const char *from); // copy a string in another string

char *strncpy(char *to, const char *from, size_t n);

#endif