#include <assert.h>
#include <unistd.h>

#include "malloc.h"

#define MEM_BLOCK_NAME_SIZE 8
#define PAGE_SIZE 4096

static void *initial_break = NULL;
static void *current_break = NULL;

typedef struct mem_block {
    // optional for debugging
    char name[MEM_BLOCK_NAME_SIZE];

    // Byte 0: 0 = free, 1 = used
    // If free then the size is used as a free list pointer
    size_t size;

    struct mem_block *next_block;
    struct mem_block *prev_block;

    void *data;
} mem_block_t;

static inline size_t
align(size_t n, int align)
{
    return (n + align - 1) & ~(align - 1);
}

void
init_malloc()
{
    initial_break = sbrk(0);
}

bool
extend_memory(size_t size)
{
    assert(size % PAGE_SIZE == 0);

    void *new_break = sbrk(size);
    if (new_break == (void *)-1) {
        return false;
    }

    current_break = new_break;

    return true;
}

void *
get_free_block(size_t size)
{
    struct mem_block *block = current_break;

    while (block) {
        if (block->size >= size) {
            return block;
        }

        block = block->next_block;
    }

    return NULL;
}

static inline void
mark_as_used_free_block(mem_block_t *block)
{
    block->size = block->size & ~1;
}

void *
malloc(size_t size)
{
    mem_block_t *block = NULL;

    size = align(size, sizeof(void *));

    block = get_free_block(size);

    if (block == NULL) {
        size_t block_size = align(size, PAGE_SIZE);

        bool ret = extend_memory(block_size);
        if (ret == false) {
            return NULL;
        }

        block = get_free_block(size);
    }

    mark_as_used_free_block(block);

    return block->data;
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
