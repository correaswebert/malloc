#include "malloc.h"

void *
malloc(size_t size)
{
    void *ptr;

    return ptr;
}

void
free(void *ptr)
{

}

bool
malloc_setfsm(enum free_space_management_algorithm fsm)
{
    // TODO: do this under lock

    fsm_algo = fsm;

    return true;
}
