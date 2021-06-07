/*
 * Copyright (C) 2019 Santiago León O.
 */

void replace_test (struct test_ctx_t *t, char *test_name, mem_pool_t *pool,
                   char *str, char *find, char *replace, char *expected, int expected_replacements)
{
    bool success = true;
    test_push (t, "%s", test_name);

    int replacements = 0xC0FEE; // initialize with garbage...
    char *res = cstr_dupreplace (pool, str, find, replace, &replacements);
    if (strcmp (res, expected) != 0 || replacements != expected_replacements) {
        str_cat_printf (t->error, "String replacement:\n");
        str_cat_printf (t->error, " src: '%s'\n", str);
        str_cat_printf (t->error, " find: '%s'\n", find);
        str_cat_printf (t->error, " replace: '%s'\n\n", replace);
        str_cat_printf (t->error, "Expected: '%s'\n     got: '%s'\n", expected, res);
        str_cat_printf (t->error, "Expected replacements: %d\n                  got: %d\n", expected_replacements, replacements);
        success = false;
    }

    test_pop (t, success);
}

void string_tests (struct test_ctx_t *t)
{
    test_push (t, "String");

    {
        string_t str = {0};
        test_push (t, "set/cat print-like");
        str_set_printf (&str, "%d, and '%s', plus %c", 120, "FOO", 'o');
        str_cat_printf (&str, ", some appended text %d", 44);
        if (strcmp (str_data(&str), "120, and 'FOO', plus o, some appended text 44") != 0) {
            test_pop (t, false);
        } else {
            test_pop (t, true);
        }
        str_free (&str);
    }

    {
        mem_pool_t pool = {0};

        replace_test (t, "Simple replacement", &pool,
                      "some X random string X",
                      "X",
                      "random",
                      "some random random string random", 2);

        replace_test (t, "Replace to empty string (delete)", &pool,
                      "some teDELETEMEst with deletemeDELETEME",
                      "DELETEME",
                      "",
                      "some test with deleteme", 2);

        replace_test (t, "Not found", &pool,
                      "a string",
                      "other stuff",
                      "",
                      "a string", 0);


        mem_pool_destroy (&pool);

        {
            {
                bool success = true;
                test_push (t, "cstr_rstrip");
                char str[] = "Hey there    ";
                cstr_rstrip (str);
                if (strcmp (str, "Hey there") != 0) { 
                    success = false;
                }
                test_pop (t, success);
            }

            {
                bool success = true;
                test_push (t, "cstr_rstrip (all spaces)");
                char str[] = "    ";
                cstr_rstrip (str);
                if (strcmp (str, "") != 0) { 
                    success = false;
                }
                test_pop (t, success);
            }

            {
                bool success = true;
                test_push (t, "cstr_rstrip (empty string)");
                char str[] = "";
                cstr_rstrip (str);
                if (strcmp (str, "") != 0) { 
                    success = false;
                }
                test_pop (t, success);
            }

            {
                bool success = true;
                test_push (t, "cstr_rstrip (no right spaces)");
                char str[] = "something";
                cstr_rstrip (str);
                if (strcmp (str, "something") != 0) { 
                    success = false;
                }
                test_pop (t, success);
            }
        }
    }

    parent_test_pop (t);
}
