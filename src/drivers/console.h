#ifndef CONSOLE_H
#define CONSOLE_H

#include <include/def.h>

/* Keyboard + VGA terminal behind the standard streams (file descriptors 0, 1, 2) */

#define CONSOLE_LINE_MAX    256
/* Ctrl+D */
#define CONSOLE_EOF         0x04

/*
    Line buffered like a canonical tty: when no input is pending a whole line is
    read from the keyboard (echoed, backspace edits it), then it's handed out
    across reads. Returns the bytes read, the line includes its '\n'.
    Ctrl+D hands out the line typed so far, 0 bytes (end of file) if it's empty.
*/
int console_read(void *buffer, size_t size);
/* Returns the bytes written */
int console_write(uint8_t color, const void *buffer, size_t size);

#endif
