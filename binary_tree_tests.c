/*
 * Copyright (C) 2019 Santiago León O.
 */
#include "binary_tree.c"

BINARY_TREE_NEW(str_int, char*, int, strcmp(a,b))

BINARY_TREE_NEW(str_string, string_t, string_t, strcmp(str_data(&a),str_data(&b)))

void binary_tree_sample ()
{
    struct str_int_t tree = {0};

    str_int_insert (&tree, "zeus", 20);
    str_int_insert (&tree, "ares", 3);
    str_int_insert (&tree, "juno", 99);

    // Node iteration
    printf ("Tree nodes:\n");
    BINARY_TREE_FOR(str_int, &tree, n)
    {
        printf (" %s: %d\n", n->key, n->value);
    }
    printf ("\n");

    // Node lookup
    // NOTE: We can't use 'n' as name for the node variable because n is used
    // previously for the iteration and it's declared at local scope. This is an
    // limitation of the API. We could also wrap the BINARY_TREE_FOR() loop in a
    // local scope and use 'n'.
    struct str_int_node_t *node = NULL;
    if (str_int_lookup (&tree, "zeus", &node)) {
        printf ("Value of 'zeus': %d\n", node->value);
    }

    str_int_destroy (&tree);
}

char **key_arrs[] = {
    (char *[]){"zeus", "ares", "juno", NULL},
    (char *[]){"Santiago", "Cash", "Institute", "Foo", "Banana", "Yule", NULL},
    (char *[]){"Santiago", "Cash", "Institute", "Foo", "Banana", "Yule", "Derek", "Darwin", NULL},

    NULL
};

bool get_test_key_list (int i, char ***arr, size_t *arr_len)
{
    assert (arr != NULL && arr_len != NULL);

    bool has_next = true;

    char **i_arr = key_arrs[i];
    size_t i_arr_len = 0;

    if (i_arr != NULL) {
        char **curr_key = i_arr;
        while (*curr_key != NULL) {
            i_arr_len++;
            curr_key++;
        }

        *arr = i_arr;
        *arr_len = i_arr_len;

    } else {
        has_next = false;
    }

    return has_next;
}

void binary_tree_tests (struct test_ctx_t *t)
{
    test_push (t, "Binary tree");
    //binary_tree_sample ();

    char **arr;
    size_t arr_len;
    for (int key_arr_idx=0; get_test_key_list (key_arr_idx, &arr, &arr_len); key_arr_idx++) {
        bool success = true;
        test_push (t, "Key arr %d", key_arr_idx);

        struct str_int_t tree = {0};
        for (int i=0; i<arr_len; i++) {
            str_int_insert (&tree, arr[i], i);
        }

        // Test all keys are being iterated
        int num_nodes_interated = 0;
        {
            test_push (t, "All nodes are iterated");
            BINARY_TREE_FOR(str_int, &tree, n)
            {
                num_nodes_interated++;
            }

            bool result = num_nodes_interated==arr_len;
            if (!result) {
                str_set_printf (t->error, "Nodes: %ld, Iterated: %d\n", arr_len, num_nodes_interated);
            }
            test_pop (t, result);
            success = success && result;
        }

        // Test keys are sorted
        {
            test_push (t, "Nodes are iterated in order");

            // TODO: There is no get successor functionality right now.
            // Implement this so we don't need the auxiliary array.
            int n_idx = 0;
            struct str_int_node_t *nodes[arr_len];
            BINARY_TREE_FOR(str_int, &tree, n)
            {
                nodes[n_idx] = n;
                n_idx++;
            }

            bool result = true;
            for (int i=0; i<num_nodes_interated-1; i++) {
                if (strcmp (nodes[i]->key, nodes[i+1]->key) > 0) {
                    result = false;
                    break;
                }
            }

            test_pop (t, result);
            success = success && result;
        }

        str_int_destroy (&tree);

        test_pop (t, success);
    }

    test_pop_parent (t);
}
