#include <libs/ctype.h>

int isalnum(int c)
{
    return isalpha(c) || isdigit(c);
}

int isalpha(int c)
{
    return isupper(c) || islower(c);
}

int iscntrl(int c)
{
    return (unsigned)c < 0x20 || c == 0x7f;
}

int isdigit(int c)
{
    return c >= '0' && c <= '9';
}

int isgraph(int c)
{
    return (unsigned)c - 0x21 < 0x5e;
}

int islower(int c)
{
    return c >= 'a' && c <= 'z';
}

int isprint(int c)
{
	return (unsigned)c - 0x20 < 0x5f;
}

int ispunct(int c)
{
	return isgraph(c) && !isalnum(c);
}

int isspace(int c)
{
	return c == ' ' || (unsigned)c - '\t' < 5;
}

int isupper(int c)
{
    return c >= 'A' && c <= 'Z';
}

int isxdigit(int c)
{
	return isdigit(c) || ((unsigned)c | 32) - 'a' < 6;
}



int isascii(int c)
{
    return !(c & ~0x7f);
}

int isblank(int c)
{
    return c == ' ' || c == '\t';
}

int toupper(int c)
{
    return c - 'a' + 'A';
}

int tolower(int c)
{
    return c - 'A' + 'a';
}