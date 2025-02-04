#include <assert.h>

#include "malloc.h"

int
main()
{
    assert(fsm_algo == FSM_FIRST_FIT);

    void *ptr = malloc(10);

    return 0;
}