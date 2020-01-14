/*
 * Copiright (C) 2019 Santiago León O.
 */
#include "common.h"
#include "linked_list.c"
#include "test_logger.c"
#include "binary_tree.c"

BINARY_TREE_NEW(str_int, char*, int, strcmp(a,b))

void binary_tree_sample ()
{
    struct str_int_tree_t tree = {0};

    str_int_tree_insert (&tree, "zeus", 20);
    str_int_tree_insert (&tree, "ares", 3);
    str_int_tree_insert (&tree, "juno", 99);

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
    struct str_int_tree_node_t *node = NULL;
    if (str_int_tree_lookup (&tree, "zeus", &node)) {
        printf ("Value of 'zeus': %d\n", node->value);
    }

    str_int_tree_destroy (&tree);
}

char **key_arrs[] = {
    (char *[]){"zeus", "ares", "juno", NULL},
    (char *[]){"Santiago", "Cash", "Institute", "Foo", "Banana", "Yule", NULL},
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

int main (int argc, char **argv)
{
    binary_tree_sample ();

    struct test_ctx_t t = {0};

    char **arr;
    size_t arr_len;
    for (int key_arr_idx=0; get_test_key_list (key_arr_idx, &arr, &arr_len); key_arr_idx++) {
        bool success = true;
        test_push (&t, "Key arr %d", key_arr_idx);

        struct str_int_tree_t tree = {0};
        for (int i=0; i<arr_len; i++) {
            str_int_tree_insert (&tree, arr[i], i);
        }

        // Test all keys are being iterated
        int num_nodes = 0;
        {
            test_push (&t, "All nodes are iterated");
            BINARY_TREE_FOR(str_int, &tree, n)
            {
                num_nodes++;
            }

            bool result = num_nodes==arr_len;
            test_pop (&t, result);
            success = success && result;
        }

        // Test keys are sorted
        {
            test_push (&t, "Nodes are iterated in order");

            // TODO: There is no get successor functionality right now.
            // Implement this so we don't need the auxiliary array.
            int n_idx = 0;
            struct str_int_tree_node_t *nodes[arr_len];
            BINARY_TREE_FOR(str_int, &tree, n)
            {
                nodes[n_idx] = n;
                n_idx++;
            }

            bool result = true;
            for (int i=0; i<num_nodes-1; i++) {
                if (strcmp (nodes[i]->key, nodes[i+1]->key) > 0) {
                    result = false;
                    break;
                }
            }

            test_pop (&t, result);
            success = success && result;
        }

        str_int_tree_destroy (&tree);

        test_pop (&t, success);
    }

    printf ("\n%s", str_data(&t.result));

    return 0;
}
