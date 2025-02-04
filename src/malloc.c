#include <assert.h>
#include <unistd.h>

#include "malloc.h"

#define MEM_BLOCK_NAME_SIZE 8
#define PAGE_SIZE 4096

static_assert(sizeof(void *) % 8 == 0, "Pointer size is not a multiple of 8");

#define BLOCK_SIZE_FLAG_MASK    0b111
#define BLOCK_FREE              0b001
#define BLOCK_PAGE_ALIGNED      0b010
#define BLOCK_LIST_HEAD         0b100

static void *current_break = NULL;

typedef struct mem_block {
    // optional for debugging
    char name[MEM_BLOCK_NAME_SIZE];

    // blocks are allocated in word sizes, which are generally greater than 8 bytes
    // so the last bit is always zero, and can be used as a flag
    // Let Byte 0 indicate LSB
    // Byte 0: 1 -> free, used to get next free block
    // Byte 1: 1 -> page aligned, used to sbrk the memory back
    size_t size;

    struct mem_block *next_block;
    struct mem_block *prev_block;
    struct mem_block *next_free_block;
} mem_block_t;

mem_block_t *free_list_head = NULL;
mem_block_t *block_list_head = NULL;

mem_block_t *
get_header(void *ptr)
{
    return (mem_block_t *)((char *)ptr - sizeof(mem_block_t));
}

static inline size_t
align(size_t n, int align)
{
    return (n + align - 1) & ~(align - 1);
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

    mem_block_t *new_block = new_break;

    new_block->next_block = NULL;
    new_block->size = size | BLOCK_FREE | BLOCK_PAGE_ALIGNED;

    new_block->next_free_block = free_list_head;
    free_list_head = new_block;

    if (block_list_head == NULL) {
        block_list_head = new_block;
        block_list_head->size |= BLOCK_LIST_HEAD;
        block_list_head->prev_block = block_list_head;
        free_list_head = block_list_head;
    } else {
        mem_block_t *list_tail = block_list_head->prev_block;
        new_block->prev_block = list_tail;
        list_tail->next_block = new_block;
        block_list_head->prev_block = new_block;
    }


    return true;
}

bool
release_memory(size_t size)
{
    assert(size % PAGE_SIZE == 0);

    void *new_break = sbrk(-size);
    if (new_break == (void *)-1) {
        return false;
    }

    current_break = new_break;

    return true;

}

mem_block_t *
get_first_fit(size_t size)
{
    mem_block_t *block = free_list_head;

    while (block) {
        if ((block->size & ~BLOCK_SIZE_FLAG_MASK) >= size) {
            return block;
        }

        block = block->next_block;
    }

    return NULL;
}

/**
 * @param size memory size to be allocated
 * @return pointer to the allocated memory
 */
static inline mem_block_t *
get_free_block(size_t size)
{
    switch (fsm_algo)
    {
    case FSM_FIRST_FIT:
        return get_first_fit(size);
    
    default:
        assert(!"Invalid FSM algorithm");
    }
}

static inline void
mark_block_used(mem_block_t *block)
{
    block->size &= ~BLOCK_FREE;
}

static inline void
mark_block_free(mem_block_t *block)
{
    block->size |= BLOCK_FREE;
}

static inline bool
is_block_free(mem_block_t *block)
{
    return block->size & BLOCK_FREE;
}

static inline bool
is_block_page_aligned(mem_block_t *block)
{
    return block->size & BLOCK_PAGE_ALIGNED;
}

void *
malloc(size_t size)
{
    mem_block_t *block = NULL;

    size = align(size + sizeof(mem_block_t), sizeof(void *));

    block = get_free_block(size);

    if (block == NULL) {
        size_t block_size = align(size, PAGE_SIZE);

        bool ret = extend_memory(block_size);
        if (ret == false) {
            return NULL;
        }

        block = get_free_block(size);
    }

    mark_block_used(block);

    return block + sizeof(mem_block_t);
}

static inline void
merge_neigh_free_blocks(mem_block_t *block)
{
    mem_block_t *next_block = block->next_block;

    if (next_block && is_block_free(next_block)) {
        block->size += next_block->size;
        block->next_block = next_block->next_block;
    }

    mem_block_t *prev_block = block->prev_block;

    if (prev_block && is_block_free(prev_block)) {
        prev_block->size += block->size;
        prev_block->next_block = block->next_block;
    }
}

void
free(void *ptr)
{
    mem_block_t *block = get_header(ptr);

    mark_block_free(block);

    merge_neigh_free_blocks(block);

    if (is_block_page_aligned(block)) {
        release_memory(block->size);
    }
}

bool
malloc_setfsm(enum free_space_management_algorithm fsm)
{
    // TODO: do this under lock

    fsm_algo = fsm;

    return true;
}
