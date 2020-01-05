/*
 * Copiright (C) 2019 Santiago León O.
 */

#include "common.h"
#include "linked_list.c"
#include "test_logger.c"

#include "string_tests.c"
#include "memory_pool_tests.c"
#include "sorting_tests.c"

// TODO: Add a CLI to select which tests get executed and which ones don't.
// TODO: Make tests silent and return true on success, on fail concatenate
// errors into a log.
int main (int argc, char **argv)
{
    memory_pool_tests ();
    printf ("\n");

    string_tests ();
    printf ("\n");

    sorting_tests ();
    
    return 0;
}
