#include <libs/array.h>

void clean_array(struct butos_array *b_array)
{
    for (size_t i=0; i < b_array->len; ++i)
        b_array->array[i] = 0;
    b_array->len = 0;
}

uint8_t butos_array_insert(struct butos_array *b_array, size_t pos, char val)
{
    if (b_array->len+1 >= b_array->maxlen)
        return 1;

    for (size_t i=b_array->len; i > pos; i--) 
        b_array->array[i] = b_array->array[i-1];

    b_array->array[pos] = val;
    b_array->len++;

    return 0;
}

uint8_t butos_array_append(struct butos_array *b_array, char val)
{
    uint8_t ret;

    ret = butos_array_insert(b_array, b_array->len, val);

    return ret;
}

uint8_t butos_array_pop(struct butos_array *b_array, size_t pos)
{
    if (pos >= b_array->maxlen)
        return 1;

    for (size_t i=pos-1; i < b_array->len; i++)
        b_array->array[i] = b_array->array[i+1];

    b_array->len--;

    return 0;
}