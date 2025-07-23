/*
 * Copyright (C) 2019 Santiago León O.
 */

struct test_structure_t {
    int i;
    float f;
    string_t *str;
    string_t str_set;
};

// Global variable to track test_callback executions during tests
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

void push_test_struct (mem_pool_t *pool, int i, float f, char *str)
{
    struct test_structure_t *test_struct = 
        (struct test_structure_t*) mem_pool_push_size_cb(pool, sizeof(struct test_structure_t), test_struct_callback);
    test_struct->str_set = str_new ("");
    test_struct->i = i;
    test_struct->f = f;

    // Test pooled strings
    test_struct->str = str_new_pooled (pool, str);
    str_set_pooled (pool, &test_struct->str_set, str);
    str_cat_c (&test_struct->str_set, "(set)");
}

void memory_pool_tests (struct test_ctx_t *t)
{
    test_push (t, "Memory Pool");

    {
        test_push (t, "Basic allocation");
        mem_pool_t pool = {0};

        int *data = mem_pool_push_struct(&pool, int);
        *data = 42;

        bool success = (*data == 42);
        test_pop (t, success);

        mem_pool_destroy (&pool);
    }

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

    {
        test_push (t, "Destroy callbacks");
        mem_pool_t pool = {0};
        g_callbacks_executed = 0;

        mem_pool_push_size_cb(&pool, 100, test_callback);
        mem_pool_push_size_cb(&pool, 200, test_callback);

        mem_pool_destroy (&pool);

        bool success = (g_callbacks_executed == 2);
        if (!success) {
            str_cat_printf (t->error, "Expected 2 callbacks, got %d\n", g_callbacks_executed);
        }
        test_pop (t, success);
    }

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

        mem_pool_destroy (&pool);

        test_pop (t, success);
    }

    {
        test_push (t, "Temporary memory callbacks");
        mem_pool_t pool = {0};
        g_callbacks_executed = 0;

        mem_pool_push_size_cb(&pool, 100, test_callback);

        mem_pool_marker_t marker = mem_pool_begin_temporary_memory(&pool);

        mem_pool_push_size_cb(&pool, 200, test_callback);
        mem_pool_push_size_cb(&pool, 300, test_callback);

        mem_pool_end_temporary_memory(marker);

        bool success = (g_callbacks_executed == 2);
        if (!success) {
            str_cat_printf (t->error, "Expected 2 temporary callbacks, got %d\n", g_callbacks_executed);
        }

        mem_pool_destroy (&pool);

        // After destroying parent, permanent callback should also execute
        success = (g_callbacks_executed == 3);
        if (!success) {
            str_cat_printf (t->error, "Expected 3 total callbacks, got %d\n", g_callbacks_executed);
        }

        test_pop (t, success);
    }

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

        mem_pool_destroy (&pool);

        test_pop (t, success);
    }

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

        mem_pool_destroy (&pool);

        test_pop (t, success);
    }

    {
        test_push (t, "Multiple bins");
        mem_pool_t pool = {0};
        pool.min_bin_size = 1024; // Small bins to force multiple

        char *data1 = mem_pool_push_array(&pool, 800, char);
        char *data2 = mem_pool_push_array(&pool, 800, char);
        char *data3 = mem_pool_push_array(&pool, 800, char);

        memset(data1, 'A', 800);
        memset(data2, 'B', 800);
        memset(data3, 'C', 800);

        bool success = (data1[0] == 'A' && data1[799] == 'A' &&
                       data2[0] == 'B' && data2[799] == 'B' &&
                       data3[0] == 'C' && data3[799] == 'C');

        if (!success) {
            str_cat_printf (t->error, "Multi-bin allocation failed\n");
        }

        mem_pool_destroy (&pool);

        test_pop (t, success);
    }

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

        mem_pool_destroy (&pool);

        test_pop (t, success);
    }

    {
        test_push (t, "Exact usage validation");
        mem_pool_t pool = {0};
        g_callbacks_executed = 0;

        size_t expected_struct_size =
            sizeof(struct test_structure_t) +
            sizeof(struct on_destroy_callback_info_t)*3 + /*test_struct_callback + 2 pooled strings*/
            sizeof(string_t);

        // We make a bin big enough to hold exactly 2 structs allocated by push_test_struct.
        pool.min_bin_size = expected_struct_size*2;

        // Verify struct sizes match expected values
        bool success = (sizeof(bin_info_t) == 32 &&
            sizeof(struct test_structure_t) == 32 &&
            sizeof(struct on_destroy_callback_info_t) == 32 &&
            sizeof(string_t) == 16 &&
            expected_struct_size == 144);

        if (!success) {
            str_cat_printf (t->error, "Struct sizes don't match expected values\n");
            str_cat_printf (t->error, "bin_info_t: %zu (expected 32)\n", sizeof(bin_info_t));
            str_cat_printf (t->error, "test_structure_t: %zu (expected 32)\n", sizeof(struct test_structure_t));
            str_cat_printf (t->error, "on_destroy_callback_info_t: %zu (expected 32)\n", sizeof(struct on_destroy_callback_info_t));
            str_cat_printf (t->error, "string_t: %zu (expected 16)\n", sizeof(string_t));
            str_cat_printf (t->error, "expected_struct_size: %zu (expected 144)\n", expected_struct_size);
        }

        push_test_struct (&pool, 10, 5.5, "This is a long string that will force a malloc");
        uint32_t allocated_after_1st = mem_pool_allocated(&pool);
        success = success && (allocated_after_1st == 320);

        if (allocated_after_1st != 320 /* 2*expected_struct_size + bin_info_t*/) {
            str_cat_printf (t->error, "After 1st struct: allocated=%u (expected 320)\n", allocated_after_1st);
            success = false;
        }

        // Begin temporary memory
        mem_pool_marker_t mrkr = mem_pool_begin_temporary_memory (&pool);

        // Allocate 2nd struct - should still fit in same bin
        push_test_struct (&pool, 4, 1.5, "And another long string");
        uint32_t allocated_after_2nd = mem_pool_allocated(&pool);
        success = success && (allocated_after_2nd == 320);

        if (allocated_after_2nd != 320) {
            str_cat_printf (t->error, "After 2nd struct: allocated=%u (expected 320)\n", allocated_after_2nd);
            success = false;
        }

        // Allocate 3rd struct - should force new bin
        push_test_struct (&pool, 20, 3.25, "bar");
        uint32_t allocated_after_3rd = mem_pool_allocated(&pool);
        success = success && (allocated_after_3rd == 640);

        if (allocated_after_3rd != 640) {
            str_cat_printf (t->error, "After 3rd struct: allocated=%u (expected 640)\n", allocated_after_3rd);
            success = false;
        }

        // End temporary memory - should call callbacks for 2nd and 3rd structs
        mem_pool_end_temporary_memory (mrkr);
        success = success && (g_callbacks_executed == 2);

        if (g_callbacks_executed != 2) {
            str_cat_printf (t->error, "After ending temporary memory: callbacks=%d (expected 2)\n", g_callbacks_executed);
            success = false;
        }

        // Second bin should've been freed
        uint32_t allocated_after_temp_end = mem_pool_allocated(&pool);
        success = success && (allocated_after_temp_end == 320);

        if (allocated_after_temp_end != 320) {
            str_cat_printf (t->error, "After temp memory end: allocated=%u (expected 320)\n", allocated_after_temp_end);
            success = false;
        }

        mem_pool_destroy (&pool); // Should call callback for 1st struct

        test_pop (t, success);
    }

    {
        test_push (t, "Child pool management");
        g_callbacks_executed = 0;
        mem_pool_t parent_pool = {0};
        mem_pool_t child_pool = {0};

        // Add data before parenting
        mem_pool_push_size_cb(&child_pool, 100, test_callback);

        mem_pool_add_child(&parent_pool, &child_pool);

        // Add data after parenting
        mem_pool_push_size_cb(&child_pool, 100, test_callback);

        // Verify child pool is functional
        int *data1 = mem_pool_push_struct(&child_pool, int);
        *data1 = 42;
        bool success = (*data1 == 42);

        // No callbacks have been called yet
        success = success && (g_callbacks_executed == 0);

        // Destroy parent pool - child pool callbacks should have been executed
        mem_pool_destroy(&parent_pool);
        success = success && (g_callbacks_executed == 2);

        if (!success) {
            str_cat_printf (t->error, "Child pool management failed\n");
            str_cat_printf (t->error, "Callbacks executed: %d (expected 2)\n", g_callbacks_executed);
            str_cat_printf (t->error, "Data values: data1=%d\n", *data1);
        }

        test_pop (t, success);
    }

    {
        test_push (t, "Nested child pools");
        mem_pool_t root_pool = {0};
        g_callbacks_executed = 0;

        // Create a hierarchy: root -> level1 -> level2
        mem_pool_t level1_pool = {0};
        mem_pool_t level2_pool = {0};
        mem_pool_add_child(&level1_pool, &level2_pool);
        mem_pool_add_child(&root_pool, &level1_pool);

        mem_pool_push_size_cb(&level1_pool, 100, test_callback);
        mem_pool_push_size_cb(&level2_pool, 200, test_callback);
        mem_pool_push_size_cb(&root_pool, 300, test_callback);

        // Verify functionality at each level
        int *root_data = mem_pool_push_struct(&root_pool, int);
        int *level1_data = mem_pool_push_struct(&level1_pool, int);
        int *level2_data = mem_pool_push_struct(&level2_pool, int);

        *root_data = 1;
        *level1_data = 2;
        *level2_data = 3;

        bool success = (*root_data == 1 && *level1_data == 2 && *level2_data == 3);

        // Verify no callbacks called yet
        success = success && (g_callbacks_executed == 0);

        // Destroy root - should cascade destroy level1, which cascades to level2
        mem_pool_destroy(&root_pool);

        // All callbacks should be executed (3 total)
        // Destruction order should be: level2 -> level1 -> root (reverse of creation)
        success = success && (g_callbacks_executed == 3);

        if (!success) {
            str_cat_printf (t->error, "Nested child pools failed\n");
            str_cat_printf (t->error, "Callbacks executed: %d (expected 3)\n", g_callbacks_executed);
        }

        test_pop (t, success);
    }

    test_pop (t, true);
}

