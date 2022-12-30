#ifndef QUEUE_H
#define QUEUE_H

#define QUEUE_EMPTY -1

/**
 * Queue implementation inspired by Jacob Sorber
 */

struct queue {
    char *values;
    int head;
    int tail;
    int size;
    int capacity;
};

typedef struct queue queue_t;

queue_t *queue_create(int max_size);
void queue_destroy(queue_t *q);
int queue_init(queue_t *q, int max_size);
void queue_fini(queue_t *q);
int queue_empty(queue_t *q);
int queue_full(queue_t *q);
int queue_enqueue(queue_t *q, char c);
char queue_dequeue(queue_t *q);


#endif
