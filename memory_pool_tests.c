/*
 * Memory pool unit tests replacing debug output.
 */

struct mp_test_struct_t {
    int value;
};

ON_DESTROY_CALLBACK(inc_counter)
{
    int *counter = clsr;
    (*counter)++;
}

void memory_pool_tests(struct test_ctx_t *t)
{
    test_push(t, "Memory Pool");

    /* Destroy callback */
    {
        test_push(t, "destroy callback");
        mem_pool_t pool = {0};
        int counter = 0;

        mem_pool_push_size_full(&pool, sizeof(struct mp_test_struct_t),
                               POOL_UNINITIALIZED,
                               inc_counter, &counter);
        mem_pool_destroy(&pool);
        bool success = counter == 1;
        if (!success) {
            test_error_current(t, "expected 1 callback, got %d\n", counter);
        }
        test_pop(t, success);
    }

    /* Temporary memory marker */
    {
        test_push(t, "temporary memory");
        mem_pool_t pool = {0};
        int counter = 0;

        mem_pool_push_size_full(&pool, sizeof(struct mp_test_struct_t),
                               POOL_UNINITIALIZED,
                               inc_counter, &counter);
        void *base_before = pool.base;
        uint32_t used_before = pool.used;
        uint32_t total_before = pool.total_data;

        mem_pool_marker_t marker = mem_pool_begin_temporary_memory(&pool);
        mem_pool_push_size_full(&pool, sizeof(struct mp_test_struct_t),
                               POOL_UNINITIALIZED,
                               inc_counter, &counter);
        mem_pool_end_temporary_memory(marker);

        bool success = counter == 1 && pool.base == base_before &&
                       pool.used == used_before &&
                       pool.total_data == total_before;
        if (!success) {
            test_error_current(t, "marker restoration failed\n");
        }
        mem_pool_destroy(&pool);
        if (counter != 2) {
            success = false;
            test_error_current(t, "expected 2 callbacks after destroy, got %d\n", counter);
        }
        test_pop(t, success);
    }

    /* Marker before pool initialization */
    {
        test_push(t, "marker before init");
        mem_pool_t pool = {0};
        int counter = 0;

        mem_pool_marker_t marker = mem_pool_begin_temporary_memory(&pool);
        mem_pool_push_size_full(&pool, sizeof(struct mp_test_struct_t),
                               POOL_UNINITIALIZED,
                               inc_counter, &counter);
        mem_pool_end_temporary_memory(marker);

        bool success = counter == 1 && pool.base == NULL && pool.size == 0 &&
                       pool.used == 0 && pool.total_data == 0;
        if (!success) {
            test_error_current(t, "pool not reset after marker\n");
        }
        mem_pool_destroy(&pool);
        if (counter != 1) {
            success = false;
            test_error_current(t, "unexpected callbacks after destroy: %d\n", counter);
        }
        test_pop(t, success);
    }

    /* Nested markers */
    {
        test_push(t, "nested markers");
        mem_pool_t pool = {0};
        int counter = 0;

        mem_pool_push_size_full(&pool, sizeof(struct mp_test_struct_t),
                               POOL_UNINITIALIZED,
                               inc_counter, &counter);
        mem_pool_marker_t m1 = mem_pool_begin_temporary_memory(&pool);
        mem_pool_push_size_full(&pool, sizeof(struct mp_test_struct_t),
                               POOL_UNINITIALIZED,
                               inc_counter, &counter);
        mem_pool_marker_t m2 = mem_pool_begin_temporary_memory(&pool);
        mem_pool_push_size_full(&pool, sizeof(struct mp_test_struct_t),
                               POOL_UNINITIALIZED,
                               inc_counter, &counter);
        mem_pool_end_temporary_memory(m2);
        bool success = counter == 1;
        mem_pool_end_temporary_memory(m1);
        success = success && counter == 2;
        mem_pool_destroy(&pool);
        success = success && counter == 3;
        if (!success) {
            test_error_current(t, "nested marker callbacks mismatch: %d\n", counter);
        }
        test_pop(t, success);
    }

    /* Child pool cleanup */
    {
        test_push(t, "child pool");
        mem_pool_t parent = {0};
        mem_pool_t child = {0};
        int counter = 0;

        mem_pool_add_child(&parent, &child);
        mem_pool_push_size_full(&child, sizeof(struct mp_test_struct_t),
                               POOL_UNINITIALIZED,
                               inc_counter, &counter);
        mem_pool_destroy(&parent);
        bool success = counter == 1;
        if (!success) {
            test_error_current(t, "child callback count %d\n", counter);
        }
        test_pop(t, success);
    }

    test_pop_parent(t);
}

