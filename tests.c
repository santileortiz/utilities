/*
 * Copyright (C) 2019 Santiago León O.
 */

#include "common.h"
#include "test_logger.c"

#include "string_tests.c"
#include "memory_pool_tests.c"
#include "linked_list_tests.c"
#include "sorting_tests.c"
#include "binary_tree_tests.c"

// TODO: Add a CLI to select which tests get executed and which ones don't.
// TODO: Make tests silent and return true on success, on fail concatenate
// errors into a log.
int main (int argc, char **argv)
{
    struct test_ctx_t t = {0};
    //t.show_all_children = true;

    // TODO: Add these to test logger
    memory_pool_tests ();
    printf ("\n");

    // TODO: Add these to test logger
    string_tests ();
    printf ("\n");

    linked_list_tests (&t);

    sorting_tests (&t);

    binary_tree_tests (&t);

    printf ("\n%s", str_data(&t.result));
    test_ctx_destroy (&t);

    return 0;
}
