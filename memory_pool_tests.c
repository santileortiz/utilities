/*
 * Copyright (C) 2019 Santiago León O.
 */

// Test data structure for callback testing
struct test_structure_t {
    int i;
    float f;
    string_t *str;
    string_t str_set;
};

// Global variable to track callback executions during tests
static int g_callbacks_executed = 0;

ON_DESTROY_CALLBACK(test_callback)
{
    g_callbacks_executed++;
}

ON_DESTROY_CALLBACK(test_struct_callback)
{
    struct test_structure_t *test = (struct test_structure_t*)allocated;
    // Verify the data is still valid when callback is called
    assert(test->i >= 0 && test->i <= 1000);
    assert(test->f >= 0.0 && test->f <= 100.0);
    g_callbacks_executed++;
}

void memory_pool_tests (struct test_ctx_t *t)
{
    test_push (t, "Memory Pool");

    // Test 1: Basic allocation
    {
        test_push (t, "Basic allocation");
        mem_pool_t pool = {0};

        int *data = mem_pool_push_struct(&pool, int);
        *data = 42;

        bool success = (*data == 42);
        test_pop (t, success);

        mem_pool_destroy (&pool);
    }

    // Test 2: Multiple allocations
    {
        test_push (t, "Multiple allocations");
        mem_pool_t pool = {0};

        int *a = mem_pool_push_struct(&pool, int);
        float *b = mem_pool_push_struct(&pool, float);
        double *c = mem_pool_push_struct(&pool, double);

        *a = 10;
        *b = 3.14f;
        *c = 2.71828;

        bool success = (*a == 10 && *b == 3.14f && *c == 2.71828);
        test_pop (t, success);

        mem_pool_destroy (&pool);
    }

    // Test 3: Callback functionality
    {
        test_push (t, "Destroy callbacks");
        mem_pool_t pool = {0};
        g_callbacks_executed = 0;

        // Allocate with callbacks
        mem_pool_push_size_cb(&pool, 100, test_callback);
        mem_pool_push_size_cb(&pool, 200, test_callback);

        mem_pool_destroy (&pool);

        bool success = (g_callbacks_executed == 2);
        if (!success) {
            str_cat_printf (t->error, "Expected 2 callbacks, got %d\n", g_callbacks_executed);
        }
        test_pop (t, success);
    }

    // Test 4: Temporary memory markers - basic
    {
        test_push (t, "Temporary memory basic");
        mem_pool_t pool = {0};

        int *permanent = mem_pool_push_struct(&pool, int);
        *permanent = 100;

        uint32_t used_before_temp = pool.used;

        mem_pool_marker_t marker = mem_pool_begin_temporary_memory(&pool);

        int *temp1 = mem_pool_push_struct(&pool, int);
        int *temp2 = mem_pool_push_struct(&pool, int);
        *temp1 = 200;
        *temp2 = 300;

        uint32_t used_during_temp = pool.used;

        mem_pool_end_temporary_memory(marker);

        uint32_t used_after_temp = pool.used;

        // Verify permanent data is still accessible and pool.used was reset
        bool success = (*permanent == 100 &&
                       used_during_temp > used_before_temp &&
                       used_after_temp == used_before_temp);
        if (!success) {
            str_cat_printf (t->error, "Temporary memory failed\n");
            str_cat_printf (t->error, "Before temp: %u, During temp: %u, After temp: %u\n",
                           used_before_temp, used_during_temp, used_after_temp);
        }
        test_pop (t, success);

        mem_pool_destroy (&pool);
    }

    // Test 5: Temporary memory with callbacks
    {
        test_push (t, "Temporary memory callbacks");
        mem_pool_t pool = {0};
        g_callbacks_executed = 0;

        // Permanent allocation with callback
        mem_pool_push_size_cb(&pool, 100, test_callback);

        mem_pool_marker_t marker = mem_pool_begin_temporary_memory(&pool);

        // Temporary allocations with callbacks
        mem_pool_push_size_cb(&pool, 200, test_callback);
        mem_pool_push_size_cb(&pool, 300, test_callback);

        mem_pool_end_temporary_memory(marker);

        // Only temporary callbacks should have been called (2)
        bool success = (g_callbacks_executed == 2);
        if (!success) {
            str_cat_printf (t->error, "Expected 2 temporary callbacks, got %d\n", g_callbacks_executed);
        }
        test_pop (t, success);

        mem_pool_destroy (&pool); // This should call the remaining permanent callback
    }

    // Test 6: Nested temporary memory
    {
        test_push (t, "Nested temporary memory");
        mem_pool_t pool = {0};

        int *permanent = mem_pool_push_struct(&pool, int);
        *permanent = 1;

        uint32_t used_after_permanent = pool.used;

        mem_pool_marker_t outer_marker = mem_pool_begin_temporary_memory(&pool);
        int *temp_outer = mem_pool_push_struct(&pool, int);
        *temp_outer = 2;

        uint32_t used_after_outer = pool.used;

        mem_pool_marker_t inner_marker = mem_pool_begin_temporary_memory(&pool);
        int *temp_inner1 = mem_pool_push_struct(&pool, int);
        int *temp_inner2 = mem_pool_push_struct(&pool, int);
        *temp_inner1 = 3;
        *temp_inner2 = 4;

        uint32_t used_max = pool.used;

        // End inner temporary memory
        mem_pool_end_temporary_memory(inner_marker);
        uint32_t used_after_inner = pool.used;

        // Verify outer temp and permanent still accessible
        bool success = (*permanent == 1 && *temp_outer == 2 &&
                       used_after_inner == used_after_outer);

        // End outer temporary memory
        mem_pool_end_temporary_memory(outer_marker);
        uint32_t used_after_outer_end = pool.used;

        success = success && (*permanent == 1 && used_after_outer_end == used_after_permanent);

        if (!success) {
            str_cat_printf (t->error, "Nested temporary memory failed\n");
            str_cat_printf (t->error, "Permanent: %u, Outer: %u, Max: %u, After inner: %u, Final: %u\n",
                           used_after_permanent, used_after_outer, used_max, used_after_inner, used_after_outer_end);
        }
        test_pop (t, success);

        mem_pool_destroy (&pool);
    }

    // Test 7: Pooled strings
    {
        test_push (t, "Pooled strings");
        mem_pool_t pool = {0};

        string_t *str1 = str_new_pooled(&pool, "Hello");
        string_t str2 = str_new("");
        str_set_pooled(&pool, &str2, "World");

        bool success = (strcmp(str_data(str1), "Hello") == 0 &&
                       strcmp(str_data(&str2), "World") == 0);

        if (!success) {
            str_cat_printf (t->error, "Pooled string failed: str1='%s', str2='%s'\n",
                           str_data(str1), str_data(&str2));
        }
        test_pop (t, success);

        mem_pool_destroy (&pool); // Should automatically free pooled strings
    }

    // Test 8: Zero-size allocation
    {
        test_push (t, "Zero-size allocation");
        mem_pool_t pool = {0};

        void *ptr = mem_pool_push_size(&pool, 0);
        bool success = (ptr == NULL);

        if (!success) {
            str_cat_printf (t->error, "Zero-size allocation should return NULL\n");
        }
        test_pop (t, success);

        mem_pool_destroy (&pool);
    }

    // Test 9: Large allocation spanning multiple bins
    {
        test_push (t, "Multiple bins");
        mem_pool_t pool = {0};
        pool.min_bin_size = 1024; // Small bins to force multiple

        // Allocate data that will span multiple bins
        char *data1 = mem_pool_push_array(&pool, 800, char);
        char *data2 = mem_pool_push_array(&pool, 800, char);
        char *data3 = mem_pool_push_array(&pool, 800, char);

        // Fill with test patterns
        memset(data1, 'A', 800);
        memset(data2, 'B', 800);
        memset(data3, 'C', 800);

        bool success = (data1[0] == 'A' && data1[799] == 'A' &&
                       data2[0] == 'B' && data2[799] == 'B' &&
                       data3[0] == 'C' && data3[799] == 'C');

        if (!success) {
            str_cat_printf (t->error, "Multi-bin allocation failed\n");
        }
        test_pop (t, success);

        mem_pool_destroy (&pool);
    }

    // Test 10: Pool statistics
    {
        test_push (t, "Pool statistics");
        mem_pool_t pool = {0};

        uint32_t initial_allocated = mem_pool_allocated(&pool);

        mem_pool_push_size_cb(&pool, 100, test_callback);
        mem_pool_push_size(&pool, 200);

        uint32_t allocated = mem_pool_allocated(&pool);
        uint32_t callback_info = mem_pool_callback_info(&pool);

        bool success = (allocated > initial_allocated && callback_info > 0);

        if (!success) {
            str_cat_printf (t->error, "Pool statistics incorrect: allocated=%u, callback_info=%u\n",
                           allocated, callback_info);
        }
        test_pop (t, success);

        mem_pool_destroy (&pool);
    }

    test_pop (t, true);
}