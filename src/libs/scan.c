#include <drivers/keyboard.h>

int getchar()
{
    while (queue_empty(input_buffer));
    return (unsigned char)(queue_dequeue(input_buffer));
}

#include <libs/print.h>
char *gets(char *s, size_t size)
{
    char c;
    char *string = s;

    if (size <= 2)
        return NULL;

    while ((c = getchar()) != -1 && c != '\n' && size-->2) {
        if (c == '\b') {
            if (s > string)
                s--;
        } else {
            *s++ = c;
        }
    }
    
    if (c != -1)
        *s++ = c;
    *s = '\0';

    return string;
}