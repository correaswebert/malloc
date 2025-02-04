#include <assert.h>

#include "malloc.h"
#include "test_malloc.h"

int
main()
{
    assert(fsm_algo == FSM_FIRST_FIT);

    malloc_print();
    void *p1 = malloc(10);
    malloc_print();
    free(p1);
    malloc_print();

    void *p2 = malloc(10);
    malloc_print();
    void *p3 = malloc(10);
    malloc_print();
    void *p4 = malloc(10);
    malloc_print();
    void *p5 = malloc(10);
    malloc_print();
    void *p6 = malloc(10);
    malloc_print();

    free(p3);
    malloc_print();

    free(p5);
    malloc_print();

    free(p2); // FIX: issue occurring here
    malloc_print();

    free(p4);
    free(p6);

    malloc_print();

    return 0;
}