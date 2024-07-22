#include <limits.h>
#include <libs/ctype.h>
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
	
    return c;
}

char *gets(char *s, size_t size)
{
    char *string = s;

    if (size <= 1)
        return NULL;
    
    while (size > 1) {
		char c;
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

/**
 * SOURCE: https://github.com/embeddedartistry/libc/blob/master/src/stdlib/strtol.c
 * Convert a string to a long integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
long strtol(nptr, endptr, base)
    const char* nptr;
    char** endptr;
    int base;
{
	const char* s = nptr;
	unsigned long acc;
	int c;
	unsigned long cutoff;
	int neg = 0;
	int any;
	int cutlim;

	// check base
	if(base < 0 || base > 36)
	{
		// errno = EINVAL

		if(endptr)
		{
			*endptr = (char*)(uintptr_t)nptr;
		}

		return 0;
	}

	/*
	 * Skip white space and pick up leading +/- sign if any.
	 * If base is 0, allow 0x for hex and 0 for octal, else
	 * assume decimal; if base is already 16, allow 0x.
	 */
	do
	{
		c = *s++;
	} while(isspace(c));

	if(c == '-')
	{
		neg = 1;
		c = *s++;
	}
	else if(c == '+')
	{
		c = *s++;
	}

	if((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X'))
	{
		c = s[1];
		s += 2;
		base = 16;
	}
	else if((base == 0 || base == 2) && c == '0' && (*s == 'b' || *s == 'B'))
	{
		c = s[1];
		s += 2;
		base = 2;
	}

	if(base == 0)
	{
		base = c == '0' ? 8 : 10;
	}

	/*
	 * Compute the cutoff value between legal numbers and illegal
	 * numbers.  That is the largest legal value, divided by the
	 * base.  An input number that is greater than this value, if
	 * followed by a legal input character, is too big.  One that
	 * is equal to this value may be valid or not; the limit
	 * between valid and invalid numbers is then based on the last
	 * digit.  For instance, if the range for longs is
	 * [-2147483648..2147483647] and the input base is 10,
	 * cutoff will be set to 214748364 and cutlim to either
	 * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
	 * a value > 214748364, or equal but the next digit is > 7 (or 8),
	 * the number is too big, and we will return a range error.
	 *
	 * Set any if any `digits' consumed; make it negative to indicate
	 * overflow.
	 */
	cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
	cutlim = (int)(cutoff % (unsigned long)base);
	cutoff /= (unsigned long)base;

	for(acc = 0, any = 0;; c = *s++)
	{
		if(isdigit(c))
		{
			c -= '0';
		}
		else if(isalpha(c))
		{
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		}
		else
		{
			break;
		}

		if(c >= base)
		{
			break;
		}

		if(any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
		{
			any = -1;
		}
		else
		{
			any = 1;
			acc *= (unsigned long)base;
			acc += (unsigned long)c;
		}
	}

	if(any < 0)
	{
		acc = neg ? (unsigned long)LONG_MIN : (unsigned long)LONG_MAX;
		//		errno = ERANGE;
	}
	else if(neg)
	{
		acc = -acc;
	}

	if(endptr != 0)
	{
		*endptr = (char*)(uintptr_t)(any ? s - 1 : nptr);
	}
	return (long)(acc);
}
