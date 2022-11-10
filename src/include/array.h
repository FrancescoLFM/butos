#ifndef ARRAY_H
#define ARRAY_H

#include <include/def.h>

struct butos_array {
    char *array;
    uint32_t len;
    uint32_t maxlen;
};

// if one of these functions fails, it will return 1
// otherwise, it will return 0

void clean_array(struct butos_array *b_array);
uint8_t butos_array_insert(struct butos_array *b_array, size_t pos, char val);
uint8_t butos_array_append(struct butos_array *b_array, char val);
uint8_t butos_array_pop(struct butos_array *b_array, size_t pos);

#endif