/*
 * Copiright (C) 2019 Santiago León O.
 */
#include "common.h"

#include "binary_tree.c"

BINARY_TREE_NEW(str_int, char*, int, strcmp(a,b))

#define BINARY_TREE_FOREACH_CB(name) void name(struct str_int_tree_node_t *node, void *data)
typedef BINARY_TREE_FOREACH_CB(str_int_tree_foreach_cb_t);

void str_int_tree_foreach (struct str_int_tree_t *tree, str_int_tree_foreach_cb_t *cb, void *data)
{
    if (tree->num_nodes < 1) return;

    mem_pool_t pool = {0};

    /* TODO: Maybe get a real stack here? seems wasteful to create an array that
     * could hold all nodes. When we implement balancing, we can make this array
     * of size log(num_nodes), just need to be sure of an upper bound.*/
    struct str_int_tree_node_t **stack =
        mem_pool_push_array (&pool, tree->num_nodes, struct str_int_tree_node_t);

    int stack_idx = 0;
    struct str_int_tree_node_t *curr_node = tree->root;
    while (true) {
        if (curr_node != NULL) {
            printf ("curr_node->key: %s\n", curr_node->key);
            stack[stack_idx++] = curr_node;
            curr_node = curr_node->left;
            printf ("Updated curr_node %p\n", curr_node);

        } else {
            if (stack_idx == 0) break;

            curr_node = stack[--stack_idx];
            cb (curr_node, data);

            curr_node = curr_node->right;
        }
    }

    mem_pool_destroy (&pool);
}

BINARY_TREE_FOREACH_CB(print_keys_cb)
{
    printf ("%s: %d\n", node->key, node->value);
}

int main (int argc, char **argv)
{
    struct str_int_tree_t tree = {0};

    str_int_tree_insert (&tree, "zeus", 20);
    str_int_tree_insert (&tree, "ares", 3);
    str_int_tree_insert (&tree, "juno", 99);

    str_int_tree_foreach (&tree, print_keys_cb, NULL);
    printf ("\n");

    printf ("Iterator:\n");

    struct str_int_tree_node_t *curr_node = (&tree)->root;
    for (struct {
             bool break_needed;
             int stack_idx;
             struct str_int_tree_node_t **stack;
         } _loop_ctx = {
             false,
             0,
             malloc ((&tree)->num_nodes*sizeof(struct str_int_tree_node_t))
         };

         _loop_ctx.break_needed = false,
         (curr_node != NULL ?
            (_loop_ctx.stack[_loop_ctx.stack_idx++] = curr_node,
             curr_node = curr_node->left,
             0)
         :
            (_loop_ctx.stack_idx == 0 ?
                (_loop_ctx.break_needed = true, 0)
            :
                (curr_node = _loop_ctx.stack[--_loop_ctx.stack_idx], 0),
            0)
         ),
         _loop_ctx.break_needed ? free (_loop_ctx.stack), false : true;

         curr_node != NULL ?
             (curr_node = curr_node->right, 0) : 0)
         if (curr_node != NULL)
         {
             printf ("%s: %d\n", curr_node->key, curr_node->value);
         }

    printf ("\n");

    BINARY_TREE_FOR(str_int, &tree, n)
    {
        printf ("%s: %d\n", n->key, n->value);
    }
    printf ("\n");

    return 0;
}
