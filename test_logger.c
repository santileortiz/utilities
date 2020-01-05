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

// TODO: Move this into common.h?
void str_cat_indented (string_t *str1, string_t *str2, int num_spaces)
{
    if (str_len(str2) == 0) {
        return;
    }

    char *c = str_data(str2);
    for (int i=0; i<num_spaces; i++) {
        strn_cat_c (str1, " ", 1);
    }

    while (c && *c) {
        if (*c == '\n' && *(c+1) != '\n' && *(c+1) != '\0') {
            strn_cat_c (str1, "\n", 1);
            for (int i=0; i<num_spaces; i++) {
                strn_cat_c (str1, " ", 1);
            }

        } else {
            strn_cat_c (str1, c, 1);
        }
        c++;
    }
}

void test_push (struct test_ctx_t *tc, char *name)
{
    struct test_t *test;

    if (tc->test_fl == NULL) {
        LINKED_LIST_PUSH_NEW (struct test_t, tc->test_stack, &tc->pool, new_test);
        test = new_test;

    } else {
        test = tc->test_fl;
        tc->test_fl = tc->test_fl->next;
        test->next = NULL;

        LINKED_LIST_PUSH (struct test_t, tc->test_stack, test);
    }

    str_set (&test->error, "");
    tc->error = &test->error;

    {
        str_set (&test->output, name);
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
        str_set (&tc->result, str_data(&curr_test->output));
    }
}
