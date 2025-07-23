/*
 * Copyright (C) 2019 Santiago León O.
 */

#define TEST_NO_SUBPROCESS

#include "common.h"
#include "test_logger.c"
#include "datetime.c"

void create_fs_tree(char *base_dir, char *entries[], int num_entries)
{
    string_t entry_path = {0};
    str_set (&entry_path, base_dir);
    str_path_ensure_ends_in_separator (&entry_path);
    int base_dir_len = str_len (&entry_path);

    if (ensure_path_exists (str_data(&entry_path))) {
        for (int i = 0; i < num_entries; i++) {
            str_put_c (&entry_path, base_dir_len, entries[i]);
            ensure_path_exists (str_data(&entry_path));

            if (str_last(&entry_path) != sys_path_sep()) {
                // Create file
                FILE *file = fopen(str_data(&entry_path), "w");
                if (!file) {
                    printf("error: unable to create file: %s", str_data(&entry_path));
                }
                fclose(file);
            }
        }
    }

    str_free (&entry_path);
}

#include "string_tests.c"
#include "path_tests.c"
#include "memory_pool_tests.c"
#include "linked_list_tests.c"
#include "sorting_tests.c"
#include "binary_tree_tests.c"
#include "datetime_tests.c"
#include "directory_iterator_tests.c"
#include "test_logger_tests.c"
#include "olc_tests.c"

// TODO: Add a CLI to select which tests get executed and which ones don't.
int main (int argc, char **argv)
{
    struct test_ctx_t t = {0};
    //t.show_all_children = true;

    // TODO: Add these to test logger
    memory_pool_tests ();

    test_logger_tests (&t);

    string_tests (&t);

    path_tests (&t);

    linked_list_tests (&t);

    sorting_tests (&t);

    binary_tree_tests (&t);

    datetime_tests (&t);

    directory_iterator_tests (&t);

    olc_tests (&t);

    printf ("\n%s", str_data(&t.result));
    test_ctx_destroy (&t);

    return 0;
}
