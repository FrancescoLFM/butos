#include <drivers/keyboard.h>
#include <libs/print.h>

/**
 * NOTA: i tab non vengono registrati per problemi di
 * sincronizzazione fra schermo e input
 */
int getchar()
{
    unsigned char c;

    do { /* ignora i tab */
        while (queue_empty(input_buffer));
        c = queue_dequeue(input_buffer);
    } while (c == '\t');

    putc(STD_COLOR, c);
    return c;
}

char *gets(char *s, size_t size)
{
    char c;
    char *string = s;

    if (size <= 1)
        return NULL;
    
    while (size > 1) {
        switch ((c = getchar())) {
        
        case '\b':
            if (s > string)
                s--, size++;
            break;
        
        default:
            *s++ = c, size--;
            break;
        }

        if (c == '\n' || c == -1)
            break;
    }
    
    *s = '\0';

    return string;
}
