/*
 * Copiright (C) 2019 Santiago León O.
 */

struct my_linked_list_t {
    int val;

    struct my_linked_list_t *next;
};

templ_sort (my_array_sort, int, *a < *b);
templ_sort_ll (my_linked_list_sort, struct my_linked_list_t, a->val < b->val);

// This could work as part of the linked list API? I think there are cases where
// we don't want to overwrite the end pointer. Which are these cases? try the
// example of creating a free list.
static inline
void ll_insert_after (struct my_linked_list_t **head,
                      struct my_linked_list_t **end,
                      struct my_linked_list_t *new_node)
{
    assert (end != NULL);

    if (head && *head == NULL) {
        assert (*end == NULL);
        *head = new_node;
        *end = new_node;
    } else {
        new_node->next = (*end)->next;
        (*end)->next = new_node;
        *end = new_node;
    }
}

void my_linked_list_append (mem_pool_t *pool,
                            struct my_linked_list_t **head,
                            struct my_linked_list_t **end,
                            int val)
{
    struct my_linked_list_t *new_node =
        mem_pool_push_struct (pool, struct my_linked_list_t);
    *new_node = ZERO_INIT(struct my_linked_list_t);

    new_node->val = val;

    ll_insert_after (head, end, new_node);
}

void my_linked_list_print (struct my_linked_list_t *list)
{
    bool is_first = true;

    struct my_linked_list_t *curr_node = list;
    printf ("[");
    while (curr_node) {
        if (is_first) {
            is_first = false;
        } else {
            printf (", ");
        }

        printf ("%d", curr_node->val);
        curr_node = curr_node->next;
    }
    printf ("]");
    printf ("\n");
}

void test_int_values (int *arr, int arr_len)
{
    if (arr == NULL) return;

    mem_pool_t pool = {0};

    printf ("Test values: ");
    array_print_full (arr, arr_len, ", ", "[", "]\n");

    struct my_linked_list_t *list = NULL, *list_end = NULL;
    for (int i=0; i<arr_len; i++) {
        my_linked_list_append (&pool, &list, &list_end, arr[i]);
    }
    printf ("Initial linked list: ");
    my_linked_list_print (list);

    my_linked_list_sort (&list, arr_len);
    printf ("Sorted linked list:  ");
    my_linked_list_print (list);

    int *array = mem_pool_push_array (&pool, arr_len, int);
    for (int i=0; i<arr_len; i++) {
        array[i] = arr[i];
    }
    printf ("Initial array: ");
    array_print_full (array, arr_len, ", ", "[", "]\n");

    my_array_sort (array, arr_len);

    printf ("Sorted array:  ");
    array_print_full (array, arr_len, ", ", "[", "]\n");

    mem_pool_destroy (&pool);
    printf ("\n");
}

struct sort_test_struct_t {
    int first;
    int second;

    struct sort_test_struct_t *next;
};

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

// To see how templ_sort is not stable you can switch the definition of
// stable_struct_sort() to use the non stable algorithm.
#if 1
templ_sort_stable (stable_struct_sort, struct sort_test_struct_t,
                   a->first <= b->first ? (a->first < b->first ? -1 : 0) : 1);
#else
templ_sort (stable_struct_sort, struct sort_test_struct_t, a->first = b->first);
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
        int curr_first = arr[0].first;
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
            LINKED_LIST_APPEND_NEW(struct sort_test_struct_t, new_list, pool, new_list_element);
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

void test_struct_sort ()
{
    struct test_ctx_t t = {0};
    mem_pool_t pool = {0};
    mem_pool_marker_t mrkr = mem_pool_begin_temporary_memory (&pool);

    {
        bool success = true;
        test_push (&t, "Stable sort test");

        struct sort_test_struct_t *arr;
        size_t arr_len = 0;
        for (int i=0; get_struct_array(&pool, i, &arr, &arr_len); i++) {
            test_push (&t, "arr%d", i);
            bool test_result = test_stable_struct_sorting (arr, arr_len, t.error);
            test_pop (&t, test_result);

            success = success && test_result;
            mem_pool_end_temporary_memory (mrkr);
        }

        test_pop (&t, success);
    }

    {
        bool success = true;
        test_push (&t, "Stable linked list sort test");

        struct sort_test_struct_t *list;
        size_t list_len;
        for (int i=0; get_struct_linked_list(&pool, i, &list, &list_len); i++) {
            test_push (&t, "arr%d", i);
            bool test_result = test_stable_linked_list_struct_sorting (list, list_len, t.error);
            test_pop (&t, test_result);

            success = success && test_result;
            mem_pool_end_temporary_memory (mrkr);
        }

        test_pop (&t, success);
    }

    printf ("%s", str_data(&t.result));

    mem_pool_destroy (&pool);
    test_ctx_destroy (&t);
}

void sorting_tests ()
{
    int test_values[] = {10, 3, 5, 20, 1};

    test_int_values (test_values, ARRAY_SIZE(test_values));
    test_int_values (test_values, 0);

    struct my_linked_list_t *empty_list = NULL;
    my_linked_list_sort (&empty_list, 0);

    // Weird cases, ideally we shouldn't get these. Maybe assert on them?
    // currently we interpret all of these as empty lists.
    my_array_sort (NULL, 0);
    my_array_sort (NULL, 10);
    my_linked_list_sort (NULL, 0);
    my_linked_list_sort (NULL, -1);
    my_linked_list_sort (NULL, 10);

    test_struct_sort ();
}
