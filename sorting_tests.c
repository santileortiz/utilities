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
}
