void test_wrapper (struct test_ctx_t *t, char *test_name, bool actual, bool expected)
{
    test_push (t, "%s", test_name);

    bool success = actual == expected;
    if (!success) {
        str_cat_printf (t->error, "Test \"%s\" failed\n", test_name);
        str_cat_printf (t->error, "Got %s but expected %s\n", actual?"true":"false", expected?"true":"false");
    }
    test_pop (t, success);
}

void test_logger_tests (struct test_ctx_t *t)
{
    test_push (t, "Test Logger");

    {
        STACK_ALLOCATE(struct test_ctx_t, test_ctx);

        test_push (test_ctx, "Root Test");

        test_wrapper (test_ctx, "Root Test 1", true, true);
        test_wrapper (test_ctx, "Root Test 2", false, true);

        {
            test_push (test_ctx, "Test Group");

            test_wrapper (test_ctx, "Test 1", true, true);
            test_wrapper (test_ctx, "Test 2", true, false);
            test_wrapper (test_ctx, "Test 3", false, false);

            test_pop_parent (test_ctx);
        }

        test_pop_parent (test_ctx);

        //printf ("%s", str_data(&test_ctx->result));

        test_push(t, "complex");
        char *expected = full_file_read (NULL, "expected/complex.txt", NULL);
        test_str_c (t, str_data(&test_ctx->result), expected);
        free (expected);

        test_ctx_destroy (test_ctx);
    }

    test_pop_parent (t);
}

