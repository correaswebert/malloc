#pragma once

#include <stdbool.h>
#include <stddef.h>

// TODO: split this into a separate header file to abstract the implementation

enum free_space_management_algorithm {
    FSM_FIRST_FIT,
    FSM_BEST_FIT,
    FSM_WORST_FIT,
};

static enum free_space_management_algorithm fsm_algo = FSM_FIRST_FIT;

/**
 * @param size size of memory to allocate
 * @return pointer to allocated memory
 */
void *malloc(size_t size);

/**
 * @param size size of memory to reallocate
 * @return pointer to allocated memory
 */
void *realloc(size_t size);

/**
 * @param ptr memory pointer to be freed
 */
void free(void *ptr);

/**
 * @param fsm free space management algorithm to use
 */
bool malloc_setfsm(enum free_space_management_algorithm fsm);

void malloc_print();

void malloc_leaks();

void malloc_scribble();
