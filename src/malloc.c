#include <unistd.h>

#include "malloc.h"

#define MEM_BLOCK_NAME_SIZE 8
#define PAGE_SIZE 4096

static void *current_break = NULL;

struct mem_block {
    // optional for debugging
    char name[MEM_BLOCK_NAME_SIZE];

    // Byte 0: 0 = free, 1 = used
    // If free then the size is used as a free list pointer
    size_t size;

    struct mem_block *next_block;
    struct mem_block *prev_block;

    void *data;
};

inline size_t
align(size_t n, int align)
{
    return (n + align - 1) & ~(align - 1);
}

void
init_malloc()
{
    current_break = sbrk(0);
}

void *
extend_memory(size_t size)
{
    size = align(size, PAGE_SIZE);

    void *ptr = sbrk(size);

    return ptr;
}

void *
malloc(size_t size)
{
    void *ptr = NULL;

    size = align(size, sizeof(void *));

    // check if enough memory is available

    // sbrk additional memory in a multiple of the system page size

    // use fsm algo to find the next free block

    // mark block as used

    return ptr;
}

void
free(void *ptr)
{
    // mark block as free

    // combine with the previous and next free blocks

    // sbrk the memory back if possible
}

bool
malloc_setfsm(enum free_space_management_algorithm fsm)
{
    // TODO: do this under lock

    fsm_algo = fsm;

    return true;
}
