#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "block.h"
#include "console.h"
#include "malloc.h"

static void *current_break = NULL;
static void *initial_break = NULL;

static unsigned long g_block_counter = 1;

pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

static mem_block_t *free_list_head = NULL;
static mem_block_t *block_list_head = NULL;

bool
extend_memory(size_t size)
{
    assert(size % PAGE_SIZE == 0);

    void *new_break = sbrk(size);
    if (new_break == (void *)-1) {
        return false;
    }

    current_break = new_break;

    if (initial_break == NULL) {
        initial_break = (char *)new_break + size;
    }

    mem_block_t *new_block = new_break;

    new_block->next_block = NULL;
    new_block->size = size - sizeof(mem_block_t);
    new_block->size |= BLOCK_FREE | BLOCK_PAGE_ALIGNED;

    new_block->next_free_block = free_list_head;
    free_list_head = new_block;

    if (block_list_head == NULL) {
        block_list_head = new_block;
        block_list_head->size |= BLOCK_LIST_HEAD;
        block_list_head->prev_block = NULL;
        free_list_head = block_list_head;
    } else {
        mem_block_t *list_tail = block_list_head->prev_block;
        new_block->prev_block = list_tail;
        list_tail->next_block = new_block;
        block_list_head->prev_block = new_block;
    }

#ifndef NDEBUG
    new_block->id = g_block_counter++;
#endif

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

    if (current_break == initial_break) {
        block_list_head = NULL;
        free_list_head = NULL;
    }

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
insert_free_block_at_head(mem_block_t *block)
{
    block->next_free_block = free_list_head;
    if (free_list_head) {
        free_list_head->prev_free_block = block;
    }
    free_list_head = block;
}

static inline void
remove_free_block(mem_block_t *block)
{
    if (block->prev_free_block) {
        (block->prev_free_block)->next_free_block = block->next_free_block;
    } else {
        free_list_head = block->next_free_block;
    }

    if (block->next_free_block) {
        (block->next_free_block)->prev_free_block = block->prev_free_block;
    }

    mark_block_used(block);
}

void
split_block(mem_block_t *block, size_t utilized_size)
{
    mem_block_t *new_block = (mem_block_t *)((char *)block + utilized_size);

    new_block->size = block_size(block) - utilized_size;
    new_block->next_block = block->next_block;
    new_block->prev_block = block;

    mark_block_free(new_block);

    remove_free_block(block); // block is now used
    insert_free_block_at_head(new_block);

    int block_flags = block->size & BLOCK_SIZE_FLAG_MASK;
    block->size = utilized_size;
    block->size |= block_flags;

    mark_block_used(block);

    block->next_block = new_block;

#ifndef NDEBUG
    new_block->id = g_block_counter++;
#endif
}

void *
malloc(size_t size)
{
    mem_block_t *block = NULL;

    size_t utilized_size = align(size + sizeof(mem_block_t), sizeof(void *));

    block = get_free_block(utilized_size);

    if (block == NULL) {
        size_t block_size = align(utilized_size, PAGE_SIZE);

        bool ret = extend_memory(block_size);
        if (ret == false) {
            return NULL;
        }

        block = get_free_block(utilized_size);
    }

    mark_block_used(block);

    if (block_size(block) > utilized_size) {
        split_block(block, utilized_size);
    }

    return (char *)block + sizeof(mem_block_t);
}

static inline mem_block_t *
merge_neigh_free_blocks(mem_block_t *block)
{
    mem_block_t *next_block = block->next_block;

    if (next_block && is_block_free(next_block)) {
        block->size += block_size(next_block);
        block->next_block = next_block->next_block;

        remove_free_block(next_block);
    }

    mem_block_t *prev_block = block->prev_block;

    if (prev_block && is_block_free(prev_block)) {
        prev_block->size += block_size(block);
        prev_block->next_block = block->next_block;

        remove_free_block(block);
        block = prev_block;
    }

    return block;
}

void
free(void *ptr)
{
    mem_block_t *block = get_header(ptr);

    mark_block_free(block);

    block = merge_neigh_free_blocks(block);
    insert_free_block_at_head(block);

    if (is_block_page_aligned(block)) {
        size_t utilized_size = block_size(block) + sizeof(mem_block_t);

        if (utilized_size >= PAGE_SIZE) {
            release_memory(utilized_size);
        }
    }
}

bool
malloc_setfsm(enum free_space_management_algorithm fsm)
{
    // TODO: do this under lock

    fsm_algo = fsm;

    return true;
}

/*
 * -- Current Memory State --
 * [BLOCK 0x7f0d774e7000-0x7f0d774e70a8] 168     [USED]  'Blk 1'
 * [BLOCK 0x7f0d774b0000-0x7f0d774b0050] 80      [USED]  'Blk 2'
 * [BLOCK 0x7f0d774af000-0x7f0d774af0a8] 168     [USED]  'Blk 3'
 *     ...
 * (list continues)
 * 
 * -- Free List --
 * [0x7f0d774e70a8] -> [0x7f0d774b0050] -> [0x7f0d774af0a8] -> (...) -> NULL
 */

void
malloc_print()
{
    char buffer[64];

    console_log("-- Current Memory State --\n");

    mem_block_t *block = block_list_head;

    while (block) {
        console_log("[BLOCK 0x");
        console_log(tohex((size_t)block, buffer));
        console_log("-0x");
        console_log(tohex((size_t)(char *)block + block_size(block), buffer));
        console_log("] ");
        console_log(ulltoa(block_size(block), buffer));
        console_log(" [");
        console_log(is_block_free(block) ? "FREE" : "USED");
        console_log("]  'Blk ");
        console_log(ulltoa(block->id, buffer));
        console_log("'\n");

        block = block->next_block;
    }

    console_log("\n-- Free List --\n");

    block = free_list_head;

    while (block) {
        console_log("[0x");
        console_log(tohex((size_t)block, buffer));
        console_log("] -> ");
        block = block->next_free_block;
    }

    console_log("NULL\n");

    console_log("\n-- List Pointers --\n");
    console_log("free list head: 0x");
    console_log(tohex((size_t)free_list_head, buffer));
    console_log("\nblock list head: 0x");
    console_log(tohex((size_t)block_list_head, buffer));

    console_log("\n-----------------------------------------------\n");
}