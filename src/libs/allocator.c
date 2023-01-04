#include <stdint.h>
#include <libs/string.h>
#include <libs/print.h> /* debug */
#include <include/asm.h> /* stop() */

#include <libs/allocator.h>

#define MEMSPACE_END(M) ((M)->start + (M)->size - 1)
#define MAX(A, B)       ((A) > (B) ? (A) : (B))
#define MIN(A, B)       ((A) < (B) ? (A) : (B))

/**
 * There is overlapping if the greatest start S isn't strictly bigger
 * than the smallest end E:
 * ----E
 *    S----
 */
static
int memspace_overlap(struct memspace *m1, struct memspace *m2)
{
    return MAX(m1->start, m2->start) <= MIN(MEMSPACE_END(m1), MEMSPACE_END(m2));
}

/**
 * Small is enclosed inside big if small size is not smaller than big start and 
 * small end is not larger than big end:
 * 
 * S----------------E
 *          S-------E
 */
static
int memspace_enclosed(struct memspace *big, struct memspace *small)
{
    return small->start >= big->start &&
        MEMSPACE_END(small) <= MEMSPACE_END(big);
}

void allocator_init(
    allocator_t *a,
    void *heap,
    size_t heap_size,
    void *registry,
    size_t reg_size /* in bytes */
)
{
    a->heap.start = heap;
    a->heap.size = heap_size;
    a->registry = registry;
    a->capacity = reg_size / sizeof(struct memspace);
    a->size = 0;
}

int allocator_full(allocator_t *a)
{
    return a->size >= a->capacity;
}

static
void allocator_register(allocator_t *a, struct memspace *m)
{
    size_t n;
    struct memspace *registry = a->registry;

    a->size++;
    n = a->size;
    while (n-->0) {
        if (n > 0 && registry[n-1].start > m->start) {
            memcpy(&registry[n], &registry[n-1], sizeof(*registry));
        } else {
            memcpy(&registry[n], m, sizeof(*registry));
            break;
        }
    }
}

static
void allocator_unregister(allocator_t *a, size_t index)
{
    struct memspace *registry = a->registry;

    for (size_t i = index; i < a->size - 1; i++) {
        memcpy(&registry[i], &registry[i+1], sizeof(*registry));
    }

    a->size--;
}

/**
 * Assumiamo il registro ordinato per valori crescenti di void *start
 * Parti dal primo indirizzo dell'heap e vedi se (indirizzo, size) ha 
 * overlapping con il primo elemento. Se lo ha, avanza oltre la fine del primo 
 * blocco e vedi se c'è overlapping con il secondo blocco ecc...
 * Fermati quando:
 * 1. trovi un blocco (indirizzo, size)  contenuto nell'heap che non si
 *    sovrappone con nessuno
 * 2. arrivi ad un indirizzo per cui: m not encloses insede heap
 * 3. superi la capacity
 */
void *allocator_alloc(allocator_t *a, size_t size)
{
    struct memspace m;
    size_t i;

    /* allocatore pieno */
    if (allocator_full(a) || size == 0)
        return NULL;

    m.size = size;
    m.start = a->heap.start;

    /* skip overlapping blocks */
    for (i = 0; i < a->size && memspace_overlap(&m, &a->registry[i]); i++)
        m.start += a->registry[i].size;

    /**
     * Check if the block we found lives all inside the heap.
     * In teoria se tutto è stato fatto bene serve farlo solo
     * se siamo oltre l'ultimo blocco (i == a->size)
     */
    if (!memspace_enclosed(&a->heap, &m))
        return NULL;

    /* puts m in the registy while preserving sorting */
    allocator_register(a, &m);
    
    return m.start;
}

static
int allocator_find_in_registry(allocator_t *a, const void *ptr)
{
    for (size_t i = 0; i < a->size; i++)
        if (a->registry[i].start == ptr)
            return i;
    return -1;
}

/**
 * Controlla se puoi espandere il blocco associato a ptr in loco.
 * Se non interseca con il blocco successivo cambia solo la size del blocco
 * di ptr; altrimenti prova ad allocare un nuovo blocco di dimensione size.
 * Se ci riesci copia il contenuto di ptr nel nuovo spazio allocato. Solo in
 * caso di successo libera ptr. Restituisci il nuovo indirizzo.
 */
void *allocator_realloc(allocator_t *a, void *ptr, size_t size)
{
    struct memspace m;
    int index;
    int can_enlarge = 0;

    index = allocator_find_in_registry(a, ptr);
    if (index < 0)
        return allocator_alloc(a, size);
    
    m.start = ptr;
    m.size = size;

    can_enlarge = 
        /* sei alla fine e hai molto spazio dopo */
        ((size_t)index == a->size-1 && memspace_enclosed(&a->heap, &m)) ||
        /* sei in mezzo ma non ti sovrapponi con il successivo */
        !memspace_overlap(&m, &a->registry[index+1]);
    
    if (can_enlarge) {
        a->registry[index].size = size;
        return m.start;
    }

    m.start = allocator_alloc(a, size);
    if (!m.start)
        return NULL;
    
    // here the MIN handles shrinking and enlargement
    memcpy(m.start, ptr, MIN(a->registry[index].size, size));
    allocator_free(a, ptr);

    return m.start;
}

static void raise_free_error()
{
    print_pm(RED, "[FATAL ERROR] INVALID FREE\n");
    stop();
}

void allocator_free(allocator_t *a, void *ptr)
{
    int index;

    index = allocator_find_in_registry(a, ptr);
    if (index < 0)
        raise_free_error();
    
    allocator_unregister(a, index);
}

void allocator_print(allocator_t *a, int verbose)
{
    if (verbose) printk(STD_COLOR, "heap start: 0x%x\n"
            "heap end: 0x%x\n"
            "heap size: %u\n"
            "\n"
            "registry size: %u\n"
            "registry capacity: %u\n"
            "registry:\n",
            a->heap.start, MEMSPACE_END(&a->heap), a->heap.size,
            a->size, a->capacity);
    
    for (size_t i = 0; i < a->size; i++)
        printk(STD_COLOR, "    - (0x%x, %u)\n",
                a->registry[i].start, a->registry[i].size);
}
