#ifndef SCAN_H
#define SCAN_H

#include <include/def.h>

/**
 * Contiene funzioni per leggere l'input e parsarlo
 */

int getchar();
char *gets(char *s, size_t size);

/* Funzioni di conversione da stringa */

long strtol(const char *nptr, char **endptr, int base);

#endif