/*
 * Copiright (C) 2019 Santiago León O.
 */

struct test_structure_t {
    int i;
    float f;
    string_t *str;
    string_t str_set;
};

ON_DESTROY_CALLBACK(print_total)
{
    struct test_structure_t *test = (struct test_structure_t*)allocated;
    printf ("Destroyed struct:\n");
    printf (" i: %d\n", test->i);
    printf (" f: %f\n", test->f);

    // These are illegal because they have already been freed!!
    // This shows why I don't like callbacks in general!
    //printf (" str: %s\n", str_data(test->str));
    //printf (" str_set: %s\n", str_data(&test->str_set));
    printf ("\n");
}

void push_test_struct (mem_pool_t *pool, int i, float f, char *str)
{
    struct test_structure_t *test_struct = 
        (struct test_structure_t*) mem_pool_push_size_cb(pool, sizeof(struct test_structure_t), print_total);
    test_struct->str_set = str_new ("");
    test_struct->i = i;
    test_struct->f = f;

    // Test pooled strings
    test_struct->str = str_new_pooled (pool, str);
    str_set_pooled (pool, &test_struct->str_set, str);
    str_cat_c (&test_struct->str_set, "(set)");
}

void memory_pool_tests ()
{
    mem_pool_t pool = {0};

    size_t expected_struct_size =
        sizeof(struct test_structure_t) +
        sizeof(struct on_destroy_callback_info_t)*3 + /*print_total() callback + 2 pooled strings*/
        sizeof(string_t);

    // We make a bin big enough to hold exactly 2 structs allocated by push_test_struct.
    pool.min_bin_size = expected_struct_size*2;

    printf ("sizeof(struct bin_info_t) = %ld\n", sizeof(bin_info_t));
    printf ("sizeof(struct test_structure_t) = %ld\n", sizeof(struct test_structure_t));
    printf ("sizeof(struct on_destroy_callback_info_t) = %ld\n", sizeof(struct on_destroy_callback_info_t));
    printf ("sizeof(string_t) = %ld\n", sizeof(string_t));
    printf ("Expected struct size = %ld\n", expected_struct_size);
    printf ("\n");

    printf ("Allocate 1st struct.\n");
    push_test_struct (&pool, 10, 5.5, "This is a long string that will force a malloc");
    mem_pool_print (&pool);
    printf ("\n");

    printf ("----TEMPORARY MEMORY START----\n");
    mem_pool_temp_marker_t mrkr = mem_pool_begin_temporary_memory (&pool);

    printf ("Allocate 2nd struct.\n");
    push_test_struct (&pool, 4, 1.5, "And another long string");
    mem_pool_print (&pool);
    printf ("\n");

    printf ("Allocate 3rd struct.\n");
    push_test_struct (&pool, 20, 3.25, "bar");
    mem_pool_print (&pool);
    printf ("\n");

    mem_pool_end_temporary_memory (mrkr);
    printf ("----TEMPORARY MEMORY END----\n");

    mem_pool_print (&pool);

    mem_pool_destroy (&pool);
}
