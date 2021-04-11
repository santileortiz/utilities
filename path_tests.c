/*
 * Copyright (C) 2021 Santiago León O.
 */

void user_path_unchanged (struct test_ctx_t *t, char *path, mem_pool_t *pool)
{
    test_push (t, "%s", path);
    char *user_path = resolve_user_path (path, pool);

    bool success = strcmp (path, user_path) == 0;
    if (!success) {
        str_cat_printf (t->error, "Expected '%s', got '%s'.\n", path, user_path);
    }

    test_pop (t, success);
}

void user_path_expanded (struct test_ctx_t *t, char *home_path, char *path, mem_pool_t *pool)
{
    string_t test_path = {0};
    str_set_printf (&test_path, "~/%s", path);

    string_t expected = {0};
    str_set_printf (&expected, "%s/%s", home_path, path);

    test_push (t, "%s", str_data (&test_path));
    char *user_path = resolve_user_path (str_data(&test_path), pool);

    bool success = strcmp (str_data(&expected), user_path) == 0;
    if (!success) {
        str_cat_printf (t->error, "Expected '%s', got '%s'.\n", str_data(&expected), user_path);
    }

    test_pop (t, success);

    user_path_unchanged (t, user_path, pool);

    str_free (&test_path);
    str_free (&expected);
}

void path_tests (struct test_ctx_t *t)
{
    test_push (t, "Path Manipulation");

    mem_pool_t pool = {0};

    {
        test_push (t, "User path resolution");
        char *home_path = getenv("HOME");

        test_push (t, "~");
        string_t expected = str_new(home_path);
        char *user_path = resolve_user_path ("~", &pool);
        test_pop (t, strcmp (str_data(&expected), user_path) == 0);

        user_path_unchanged (t, "~file (1).docx", &pool);

        user_path_expanded (t, home_path, "", &pool);
        user_path_expanded (t, home_path, "Downloads/file (1).docx", &pool);
        user_path_expanded (t, home_path, "Downloads/~/~file", &pool);
        user_path_expanded (t, home_path, "Downloads/.//../file.docx", &pool);

        str_free (&expected);
        parent_test_pop (t);
    }

    mem_pool_destroy (&pool);

    parent_test_pop (t);
}
