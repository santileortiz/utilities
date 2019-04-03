/*
 * Copiright (C) 2019 Santiago León O.
 */

#include "common.h"

struct test_structure_t {
    int i;
    float f;
    string_t *str;
};

void print_total (void *allocated)
{
    struct test_structure_t *test = (struct test_structure_t*)allocated;
    printf ("%f\n", test->i + test->f);
}

void push_test_struct (mem_pool_t *pool, int i, float f, char *str)
{
    struct test_structure_t *test_struct = 
        (struct test_structure_t*) mem_pool_push_size_cb(pool, sizeof(struct test_structure_t), print_total);
    test_struct->i = i;
    test_struct->f = f;
    test_struct->str = str_new_pooled (pool, str);
}

int main (int argc, char **argv)
{
    // Testing memory pools in a bootstrapped struct
    mem_pool_t pool = {0};

    pool.min_bin_size = (sizeof(struct test_structure_t) + sizeof(struct on_destroy_callback_info_t))*2;

    push_test_struct (&pool, 10, 5.5, "This a long string that will force a malloc.");

    printf ("----TEMPORARY MEMORY START----\n");
    mem_pool_temp_marker_t mrkr = mem_pool_begin_temporary_memory (&pool);
    push_test_struct (&pool, 4, 1.5, "And another long string.");
    push_test_struct (&pool, 20, 3.25, "bar");
    mem_pool_print (&pool);
    printf ("\n");
    mem_pool_end_temporary_memory (mrkr);
    printf ("----TEMPORARY MEMORY END----\n");

    mem_pool_print (&pool);
    printf ("\n");

    mem_pool_destroy (&pool);

    // Quick test for string functions with the printf syntax
    string_t str = {0};
    str_set_printf (&str, "%d, and '%s', plus %c", 120, "FOO", 'o');
    str_cat_printf (&str, ", some appended text %d", 44);
    printf ("%s\n", str_data(&str));
    str_free (&str);

    
    return 0;
}
