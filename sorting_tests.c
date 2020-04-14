/*
 * Copiright (C) 2019 Santiago León O.
 */

templ_sort (my_ascending_sort, int, *a < *b);
templ_sort (my_descending_sort, int, *a > *b);

void str_cat_array (string_t *str, int *arr, size_t arr_len)
{
    str_cat_printf (str, "{");
    for (int i=0; i<arr_len; i++) {
        str_cat_printf (str, "%i", arr[i]);

        if (i < arr_len-1) {
            str_cat_printf (str, ", ");
        }
    }
    str_cat_printf (str, "}\n");
}

bool test_int_array (int *arr, size_t arr_len, string_t *error)
{
    bool success = true;
    str_cat_c (error, "Original: ");
    str_cat_array (error, arr, arr_len);

    my_ascending_sort (arr, arr_len);
    for (int i=0; i<arr_len-1; i++) {
        if (arr[i] > arr[i+1]) {
            success = false;
        }
    }
    str_cat_c (error, "Ascending: ");
    str_cat_array (error, arr, arr_len);

    my_descending_sort (arr, arr_len);
    for (int i=0; i<arr_len-1; i++) {
        if (arr[i] < arr[i+1]) {
            success = false;
        }
    }
    str_cat_c (error, "Descending: ");
    str_cat_array (error, arr, arr_len);

    return success;
}

struct sort_test_struct_t {
    int first;
    int second;

    struct sort_test_struct_t *next;
};

templ_sort_ll (linked_list_sort, struct sort_test_struct_t, a->first < b->first);

void str_cat_struct_array (string_t *str, struct sort_test_struct_t *arr, size_t arr_len)
{
    str_cat_printf (str, "{");
    for (int i=0; i<arr_len; i++) {
        str_cat_printf (str, "{%i, %i}", arr[i].first, arr[i].second);

        if (i < arr_len-1) {
            str_cat_printf (str, ", ");
        }
    }
    str_cat_printf (str, "}\n");
}

void str_cat_struct_linked_list (string_t *str, struct sort_test_struct_t *list)
{
    str_cat_printf (str, "{");
    struct sort_test_struct_t *curr_node = list;
    while (curr_node != NULL) {
        str_cat_printf (str, "{%i, %i}", curr_node->first, curr_node->second);

        if (curr_node->next != NULL) {
            str_cat_printf (str, ", ");
        }
        curr_node = curr_node->next;
    }
    str_cat_printf (str, "}\n");
}

bool test_linked_list_sort (struct sort_test_struct_t *list, size_t list_len, string_t *error)
{
    bool success = true;

    str_cat_c (error, "Original: ");
    str_cat_struct_linked_list (error, list);

    linked_list_sort (&list, list_len);
    struct sort_test_struct_t *curr_node = list;
    while (curr_node->next != NULL) {
        if (curr_node->first > curr_node->next->first) {
            success = false;
        }

        curr_node = curr_node->next;
    }

    str_cat_c (error, "Ascending: ");
    str_cat_struct_linked_list (error, list);

    return success;
}

// To see how templ_sort is not stable you can switch the definition of
// stable_struct_sort() to use the non stable algorithm.
#if 1
templ_sort_stable (stable_struct_sort, struct sort_test_struct_t,
                   a->first <= b->first ? (a->first < b->first ? -1 : 0) : 1);
#else
templ_sort (stable_struct_sort, struct sort_test_struct_t, a->first < b->first);
#endif

bool test_stable_struct_sorting (struct sort_test_struct_t *arr, size_t arr_len, string_t *error)
{
    bool success = true;

    stable_struct_sort (arr, arr_len);
    for (int i=0; i<arr_len-1; i++) {
        if (arr[i].first > arr[i+1].first) {
            success = false;
        }
    }

    if (!success) {
        str_cat_c (error, "Failed to sort struct array:\n ");
        str_cat_struct_array (error, arr, arr_len);
    }


    if (success) {
        for (int i=0; i<arr_len-1; i++) {
            if (arr[i].first == arr[i+1].first) {
                if (arr[i].second > arr[i+1].second) {
                    success = false;
                }
            }
        }

        if (!success) {
            str_cat_c (error, "Sorting of struct array was not stable:\n ");
            str_cat_struct_array (error, arr, arr_len);
        }
    }

    return success;
}

templ_sort_stable_ll (stable_struct_linked_list_sort, struct sort_test_struct_t,
                      a->first <= b->first ? (a->first < b->first ? -1 : 0) : 1);

bool test_stable_linked_list_struct_sorting (struct sort_test_struct_t *list, size_t list_len, string_t *error)
{
    bool success = true;

    stable_struct_linked_list_sort (&list, list_len);
    struct sort_test_struct_t *curr_node = list;
    while (curr_node->next != NULL) {
        if (curr_node->first > curr_node->next->first) {
            success = false;
        }

        curr_node = curr_node->next;
    }

    if (!success) {
        str_cat_c (error, "Failed to sort linked list:\n ");
        str_cat_struct_linked_list (error, list);
    }


    if (success) {
        struct sort_test_struct_t *curr_node = list;
        while (curr_node->next != NULL) {
            if (curr_node->first == curr_node->next->first) {
                if (curr_node->second > curr_node->next->second) {
                    success = false;
                }
            }

            curr_node = curr_node->next;
        }

        if (!success) {
            str_cat_c (error, "Sorting of linked list was not stable:\n ");
            str_cat_struct_linked_list (error, list);
        }
    }

    return success;
}

bool get_test_array (mem_pool_t *pool, int i, int **arr, size_t *arr_len) {
        int arr0[] = {10, 10, 30, 40, 80, 90, 10, 90, 80, 10, 30, 20, 10, 40, 10, 20};

        int arr1[] = {10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};

        bool has_next = true;
        if (i == 0) {
            int *new_arr = mem_pool_push_array (pool, ARRAY_SIZE(arr0), int);
            for (int j=0; j<ARRAY_SIZE(arr0); j++) {
                new_arr[j] = arr0[j];
            }

            *arr = new_arr;
            *arr_len = ARRAY_SIZE(arr0);

        } else if (i == 1) {
            int *new_arr = mem_pool_push_array (pool, ARRAY_SIZE(arr1), int);
            for (int j=0; j<ARRAY_SIZE(arr1); j++) {
                new_arr[j] = arr1[j];
            }

            *arr = new_arr;
            *arr_len = ARRAY_SIZE(arr1);

        } else {
            has_next = false;
        }

        return has_next;
}

bool get_struct_array (mem_pool_t *pool, int i, struct sort_test_struct_t **arr, size_t *arr_len)
{
    bool has_next = false;
    mem_pool_t pool_l = {0};

    int *int_arr;
    size_t int_arr_len;
    if (get_test_array (&pool_l, i, &int_arr, &int_arr_len)) {
        struct sort_test_struct_t *struct_arr = mem_pool_push_array (pool, int_arr_len, struct sort_test_struct_t);
        for (int i=0; i<int_arr_len; i++) {
            struct_arr[i].first = int_arr[i];
            struct_arr[i].second = i+1;
        }

        *arr = struct_arr;
        *arr_len = int_arr_len;

        has_next = true;
    }

    mem_pool_destroy (&pool_l);

    return has_next;
}

bool get_struct_linked_list (mem_pool_t *pool, int i, struct sort_test_struct_t **list, size_t *list_len)
{
    bool has_next = false;
    mem_pool_t pool_l = {0};

    int *int_arr;
    size_t int_arr_len;
    if (get_test_array (&pool_l, i, &int_arr, &int_arr_len)) {
        struct sort_test_struct_t *new_list = NULL;
        struct sort_test_struct_t *new_list_end = NULL;
        
        for (int i=0; i<int_arr_len; i++) {
            LINKED_LIST_APPEND_NEW(pool, struct sort_test_struct_t, new_list, new_list_element);
            new_list_element->first = int_arr[i];
            new_list_element->second = i+1;
        }

        *list = new_list;
        *list_len = int_arr_len;

        has_next = true;
    }

    mem_pool_destroy (&pool_l);

    return has_next;
}

void sorting_tests (struct test_ctx_t *t)
{
    test_push (t, "Sorting");

    mem_pool_t pool = {0};
    mem_pool_marker_t mrkr = mem_pool_begin_temporary_memory (&pool);

    {
        bool success = true;
        test_push (t, "Empty list representations");

        CRASH_TEST(success, t->error,
            struct sort_test_struct_t *empty_list = NULL;
            linked_list_sort (&empty_list, 0);

            // Weird cases, ideally we shouldn't get these. Maybe assert on them?
            // currently we interpret all of these as empty lists.
            my_ascending_sort (NULL, 0);
            my_ascending_sort (NULL, 10);
            linked_list_sort (NULL, 0);
            linked_list_sort (NULL, -1);
            linked_list_sort (NULL, 10);
        )
        test_pop (t, success);
    }


    {
        test_push (t, "Array sorting (int)");

        int *arr;
        size_t arr_len = 0;
        for (int i=0; get_test_array(&pool, i, &arr, &arr_len); i++) {
            test_push (t, "arr%d", i);
            bool test_result = test_int_array (arr, arr_len, t->error);
            test_pop (t, test_result);

            mem_pool_end_temporary_memory (mrkr);
        }

        parent_test_pop (t);
    }

    {
        test_push (t, "Linked List sorting");

        struct sort_test_struct_t *arr;
        size_t arr_len = 0;
        for (int i=0; get_struct_linked_list(&pool, i, &arr, &arr_len); i++) {
            test_push (t, "arr%d", i);
            bool test_result = test_linked_list_sort (arr, arr_len, t->error);
            test_pop (t, test_result);

            mem_pool_end_temporary_memory (mrkr);
        }

        parent_test_pop (t);
    }

    {
        test_push (t, "Stable sort test");

        struct sort_test_struct_t *arr;
        size_t arr_len = 0;
        for (int i=0; get_struct_array(&pool, i, &arr, &arr_len); i++) {
            test_push (t, "arr%d", i);
            bool test_result = test_stable_struct_sorting (arr, arr_len, t->error);
            test_pop (t, test_result);

            mem_pool_end_temporary_memory (mrkr);
        }

        parent_test_pop (t);
    }

    {
        test_push (t, "Stable linked list sort test");

        struct sort_test_struct_t *list;
        size_t list_len;
        for (int i=0; get_struct_linked_list(&pool, i, &list, &list_len); i++) {
            test_push (t, "arr%d", i);
            bool test_result = test_stable_linked_list_struct_sorting (list, list_len, t->error);
            test_pop (t, test_result);

            mem_pool_end_temporary_memory (mrkr);
        }

        parent_test_pop (t);
    }

    mem_pool_destroy (&pool);

    parent_test_pop (t);
}
