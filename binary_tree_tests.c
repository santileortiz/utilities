/*
 * Copiright (C) 2019 Santiago León O.
 */
#include "common.h"
#include "binary_tree.c"

BINARY_TREE_NEW(str_int, char*, int, strcmp(a,b))

int main (int argc, char **argv)
{
    struct str_int_tree_t tree = {0};

    str_int_tree_insert (&tree, "zeus", 20);
    str_int_tree_insert (&tree, "ares", 3);
    str_int_tree_insert (&tree, "juno", 99);

    // Node iteration
    BINARY_TREE_FOR(str_int, &tree, n)
    {
        printf ("%s: %d\n", n->key, n->value);
    }
    printf ("\n");

    // Node lookup
    // NOTE: We can't use 'n' as name for the node variable because n is used
    // previously for the iteration and it's declared at local scope. This is an
    // limitation of the API. We could also wrap the BINARY_TREE_FOR() loop in a
    // local scope and use 'n'.
    struct str_int_tree_node_t *node = NULL;
    if (str_int_tree_lookup (&tree, "zeus", &node)) {
        printf ("zeus: %d\n", node->value);
    }

    str_int_tree_destroy (&tree);

    return 0;
}
