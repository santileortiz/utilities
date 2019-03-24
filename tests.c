/*
 * Copiright (C) 2019 Santiago León O.
 */

#include "common.h"

struct test_structure_t {
    int i;
    float f;
};

void print_total (void *allocated)
{
    struct test_structure_t *test = (struct test_structure_t*)allocated;
    printf ("%f\n", test->i + test->f);
}

void push_test_struct (mem_pool_t *pool, int i, float f)
{
    struct test_structure_t *test_struct = 
        (struct test_structure_t*) mem_pool_push_size_cb(pool, sizeof(struct test_structure_t), print_total);
    test_struct->i = i;
    test_struct->f = f;
}

int main (int argc, char **argv)
{
    // Testing memory pools in a bootstrapped struct
    mem_pool_t pool = {0};

    push_test_struct (&pool, 10, 5.5);
    push_test_struct (&pool, 4, 1.5);
    push_test_struct (&pool, 20, 3.25);

    mem_pool_destroy (&pool);
    
    return 0;
}
