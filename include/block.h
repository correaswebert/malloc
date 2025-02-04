#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#define PAGE_SIZE 4096
#define BLOCK_ALIGN sizeof(void *)

static_assert(BLOCK_ALIGN % 8 == 0, "Block alignment is not a multiple of 8");

#define BLOCK_SIZE_FLAG_MASK 0b111
#define BLOCK_FREE 0b001
#define BLOCK_PAGE_ALIGNED 0b010
#define BLOCK_LIST_HEAD 0b100

typedef struct mem_block
{
    // optional for debugging
    unsigned long id;

    // blocks are allocated in word sizes, which are generally greater than 8 bytes
    // so the last bit is always zero, and can be used as a flag
    // Let Byte 0 indicate LSB
    // Byte 0: 1 -> free, used to get next free block
    // Byte 1: 1 -> page aligned, used to sbrk the memory back
    size_t size;

    struct mem_block *next_block;
    struct mem_block *prev_block;

    struct mem_block *next_free_block;
    struct mem_block *prev_free_block;
} mem_block_t;

static inline mem_block_t *
get_header(void *ptr)
{
    return (mem_block_t *)((char *)ptr - sizeof(mem_block_t));
}

static inline size_t
align(size_t n, int align) {
    return (n + align - 1) & ~(align - 1);
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

static inline size_t
block_size(mem_block_t *block)
{
    return block->size & ~BLOCK_SIZE_FLAG_MASK;
}
