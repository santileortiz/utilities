/*
 * Copyright (C) 2022 Santiago León O.
 */

void directory_iterator_tests (struct test_ctx_t *t)
{
    char *test_dir[] = {
        "file.c",
        "X_dir4/dir5/",
        "X_dir3/file1",
        "X_dir3/file0",
        "X_dir2/file2",
        "X_dir2/file15",
        "X_dir2/fileC",
        "X_dir2/fileD",
        "X_dir2/fileB",
        "X_dir2/fileA",
        "A_dir1/file_B",
        "A_dir1/file_A",
        "A_dir1/file_a",
    };

    char *expected_dir[] = {
        "A_dir1/",
        "X_dir2/",
        "X_dir3/",
        "X_dir4/",
        "file.c",
        "A_dir1/file_A",
        "A_dir1/file_B",
        "A_dir1/file_a",
        "X_dir2/file2",
        "X_dir2/file15",
        "X_dir2/fileA",
        "X_dir2/fileB",
        "X_dir2/fileC",
        "X_dir2/fileD",
        "X_dir3/file0",
        "X_dir3/file1",
        "X_dir4/dir5/",
    };

    char *base_dir = "bin/directory_iterator_test";
    create_fs_tree (base_dir, test_dir, ARRAY_SIZE(test_dir));

    test_push (t, "Directory Iterator");

    {
        bool size_error = false;
        test_push (t, "Sorted");
        int i=0;
        string_t path = {0};
        string_t expected = {0};
        PATH_FOR_SORTED (base_dir, it) {
            str_set (&path, str_data(&it.path));
            if (it.is_dir) {
                str_cat_c (&path, "/");
            }

            if (i >= ARRAY_SIZE(expected_dir)) {
                size_error = true;
                break;
            }

            str_set (&expected, base_dir);
            str_cat_path (&expected, expected_dir[i]);

            test_str_small (t, NULL, str_data(&path), str_data(&expected));

            i++;
        }

        test_push (t, "equal size");
        test_bool_c (t, !size_error);

        str_free (&path);
        str_free (&expected);
        test_pop_parent (t);
    }

    {
        test_push (t, "Unsorted");

        // Oversized just in case there is an error and expected_dir array is smaller
        char *result[2*ARRAY_SIZE(expected_dir)];
        int result_len = 0;
        {
            int i = 0;
            string_t path = {0};
            PATH_FOR (base_dir, it) {
                str_set (&path, str_data(&it.path));
                if (it.is_dir) {
                    str_cat_c (&path, "/");
                }
                result[i] = strdup (str_data(&path));
                i++;
            }
            result_len = i;
            qsort(result, result_len, sizeof(result[0]), strcmp_cb);
            str_free (&path);
        }

        char *expected[ARRAY_SIZE(expected_dir)];
        {
            string_t expected_str = {0};
            for (int j=0; j<ARRAY_SIZE(expected); j++) {
                str_set(&expected_str, base_dir);
                str_cat_path (&expected_str, expected_dir[j]);

                expected[j] = strdup(str_data(&expected_str));
            }
            qsort(expected, ARRAY_SIZE(expected), sizeof(expected[0]), strcmp_cb);
            str_free (&expected_str);
        }

        bool success = test_int (t, "equal size", result_len, ARRAY_SIZE(expected_dir));

        if (success) {
            test_push (t, "equal content");
            for (int j=0; j<MIN(result_len, ARRAY_SIZE(expected_dir)); j++) {
                test_str_small (t, NULL, result[j], expected[j]);
            }
            test_pop_parent (t);
        }

        for (int j=0; j<result_len; j++) free(result[j]);
        for (int j=0; j<ARRAY_SIZE(expected); j++) free(expected[j]);
        test_pop_parent (t);
    }

    path_rmrf (base_dir);
    test_pop_parent (t);
}
