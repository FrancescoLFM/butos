#include <include/def.h>
#include <libs/alloc.h>
#include <libs/queue.h>

/**
 * Nelle funzioni che restituiscono errore
 * 1 > errore
 * 0 > successo
 */

queue_t *queue_create(int max_size)
{
    queue_t *q;
    int error = 0;
    
    q = kalloc(sizeof(*q));
    if (!q)
        return NULL;
    
    error = queue_init(q, max_size);
    if (error) {
        kfree(q);
        q = NULL;
    }
    
    return q;
}

void queue_destroy(queue_t *q)
{
	queue_fini(q);
    kfree(q);
}

int queue_init(queue_t *q, int max_size)
{
    q->capacity = max_size;
    q->values = kalloc(sizeof(*q->values) * q->capacity);
    if (q->values == NULL)
        return 1;
    q->size = 0;
    q->head = 0;
    q->tail = 0;
    return 0;
}

void queue_fini(queue_t *q)
{
    kfree(q->values);
}

int queue_empty(queue_t *q)
{
    return q->size <= 0;
}

int queue_full(queue_t *q)
{
    return q->size >= q->capacity;
}

int queue_enqueue(queue_t *q, char c)
{
    if (queue_full(q))
        return 1;
    
    q->values[q->tail] = c;
    q->size++;
    q->tail = (q->tail + 1) % q->capacity;
    
    return 0;
}

/**
 * Ci aspettiamo che il client verifichi se la coda
 * è vuota o meno
 */
char queue_dequeue(queue_t *q)
{
    int result;
    
    if (queue_empty(q))
        return QUEUE_EMPTY;
        
    result = q->values[q->head];
    q->head = (q->head + 1) % q->capacity;
    q->size--;
    
    return result;
}

void queue_flush(queue_t *q)
{
    q->size = 0;
    q->head = 0;
    q->tail = 0;
}