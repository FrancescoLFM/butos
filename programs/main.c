volatile char* cursor = (volatile char*)0xb8000;

void print_pm(int color, char* string);

char* carriage_return(char volatile* cursor);

void main()
{
    for (int i = 0; i < 25; i++)
        print_pm(0b00000111, "Lanfredi gay, ma a 32 bit\n\r");
    for(;;);
}

void print_pm(int color, char* string)
{
    do {
        switch (*string) {
        case '\n':
            cursor += 160;
            break;
        
        case '\r':
            cursor = carriage_return(cursor);
            break;
        
        default:
            *cursor++ = *string;
            *cursor++ = color;
        }
    } while (*string++);
}

char* carriage_return(char volatile* cursor)
{
    unsigned long temp = (unsigned long)(cursor) - 0xb8000;
    temp -= temp % 160;
    return (char*)(temp + 0xb8000) - 2;
}