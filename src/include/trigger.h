#ifndef TRIGGER_H
#define TRIGGER_H

typedef unsigned char trigger_t;

static inline
void trigger_set(trigger_t *trigger)
{
    *trigger = 1;
}

static inline
int trigger_is_set(trigger_t *trigger)
{
    if (*trigger == 0)
        return 0;
    
    *trigger = 0;
    return 1;
}

typedef unsigned char flag_t;

static inline
void flag_set(flag_t *flag)
{
    *flag = 1;
}

static inline
void flag_clear(flag_t *flag)
{
    *flag = 0;
}

#endif