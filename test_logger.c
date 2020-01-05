/*
 * Copiright (C) 2020 Santiago León O.
 */

struct test_t {
    string_t output;
    string_t error;
    string_t children;

    struct test_t *next;
};

// TODO: Put these inside struct test_ctx_t while still supporting zero
// initialization of the structure.
#define TEST_NAME_WIDTH 40
#define TEST_INDENT 4

struct test_ctx_t {
    mem_pool_t pool;
    string_t result;

    string_t *error;

    // By default results of child tests are only shown if the parent test
    // failed. If the following is true, then all child test results will be
    // show.
    bool show_all_children;

    struct test_t *test_stack;

    struct test_t *test_fl;
};

void test_ctx_destroy (struct test_ctx_t *tc)
{
    str_free (&tc->result);
    mem_pool_destroy (&tc->pool);
}

GCC_PRINTF_FORMAT(2, 3)
void test_push (struct test_ctx_t *tc, char *name_format, ...)
{
    struct test_t *test;
    if (tc->test_fl == NULL) {
        LINKED_LIST_PUSH_NEW (struct test_t, tc->test_stack, &tc->pool, new_test);
        str_set_pooled (&tc->pool, &new_test->error, "");
        str_set_pooled (&tc->pool, &new_test->output, "");
        str_set_pooled (&tc->pool, &new_test->children, "");

        test = new_test;

    } else {
        test = tc->test_fl;
        tc->test_fl = tc->test_fl->next;
        test->next = NULL;

        LINKED_LIST_PUSH (struct test_t, tc->test_stack, test);
    }

    str_set (&test->children, "");
    str_set (&test->error, "");
    tc->error = &test->error;

    {
        PRINTF_INIT(name_format, name_size, vargs);
        str_maybe_grow (&test->output, name_size-1, false);
        char *dst = str_data(&test->output);
        PRINTF_SET (dst, name_size, name_format, vargs);

        str_cat_c (&test->output, " ");
        while (str_len(&test->output) < TEST_NAME_WIDTH-1) {
            str_cat_c (&test->output, ".");
        }
        str_cat_c (&test->output, " ");
    }
}

void test_pop (struct test_ctx_t *tc, bool success)
{
    struct test_t *curr_test = tc->test_stack;
    tc->test_stack = tc->test_stack->next;
    curr_test->next = NULL;

    LINKED_LIST_PUSH (struct test_t, tc->test_fl, curr_test);

    if (success) {
        str_cat_c (&curr_test->output, ECMA_GREEN("OK")"\n");
    } else {
        str_cat_c (&curr_test->output, ECMA_RED("FAILED")"\n");
        str_cat_indented (&curr_test->output, &curr_test->error, TEST_INDENT);
    }

    if (tc->show_all_children || !success) {
        str_cat (&curr_test->output, &curr_test->children);
    }

    if (tc->test_stack) {
        str_cat_indented (&tc->test_stack->children, &curr_test->output, TEST_INDENT);
    } else {
        str_cat (&tc->result, &curr_test->output);
    }
}
