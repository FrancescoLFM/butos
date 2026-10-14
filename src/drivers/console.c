#include <drivers/console.h>
#include <libs/print.h>
#include <libs/scan.h>
#include <libs/string.h>

static char console_line[CONSOLE_LINE_MAX];
static size_t console_line_len;
/* Next byte of console_line to hand out */
static size_t console_line_pos;

static void console_line_fill(void)
{
    char c;

    console_line_len = 0;
    console_line_pos = 0;
    for (;;) {
        c = (char) getchar();
        /* Ends the line without a newline: on an empty line read returns 0 (end of file) */
        if (c == CONSOLE_EOF)
            return;
        if (c == '\b') {
            if (console_line_len > 0) {
                console_line_len--;
                putc(c);
            }
            continue;
        }
        /* The last byte is kept for the newline */
        if (c != '\n' && console_line_len >= CONSOLE_LINE_MAX - 1)
            continue;
        console_line[console_line_len++] = c;
        putc(c);
        if (c == '\n')
            return;
    }
}

int console_read(void *buffer, size_t size)
{
    size_t count;

    if (size == 0)
        return 0;
    if (console_line_pos == console_line_len)
        console_line_fill();
    /* Ctrl+D on an empty line */
    if (console_line_pos == console_line_len)
        return 0;

    count = console_line_len - console_line_pos;
    if (size < count)
        count = size;
    memcpy(buffer, console_line + console_line_pos, count);
    console_line_pos += count;

    return (int) count;
}

int console_write(uint8_t color, const void *buffer, size_t size)
{
    const char *str = buffer;

    for (size_t i = 0; i < size; i++)
        putc_c(color, str[i]);

    return (int) size;
}
