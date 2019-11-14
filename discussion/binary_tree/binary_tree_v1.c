/*
 * Copiright (C) 2019 Santiago León O.
 */

// This is the first generic binary tree API that I thought was actually useful.
// It allows choosing the key and value types as well as the key comparison
// function as a macro.
//
// Features:
//
//  - Single line instantiation that allows choosing prefix, key type, key comparison
//    function and value type.
//
//  - Simple generic and closure-less iteration over node elements.
//
// Drawbacks:
//
//  - When iterating, the 'node' variable is created in the local scope, so we
//    can't call 2 iteration loops in the same scope that use the same varible
//    name. I don't really see a way of fixing this because C doesn't allow
//    different variable types in the initialization expression of for loops.
//    The user will need to either rename the variable in the loop, or wrap the
//    iteration in a local scope. I don't think this is much of a problem
//    compared to the usefulness of the single line instantiation and for-like
//    iteration.
//
//  - It would be nice to be able to iterate over nodes without asking for the
//    prefix. I don't see how to fix this.
//
//  - The key comparison macro requires 3 states. This is different than the
//    current sort API where we only ask for the user to tell when A is less
//    than B. Here we need to also distinguish when A is equal to B for 2
//    reasons, 1) to stop the key lookup and 2) so we don't overwrite pointers
//    and maybe leak memory. I think we may need to do the same for the sort API
//    if we want to guarantee a stable sort, but it's very convenient to be able
//    to just do '*a<*b'.
//
//    One thing we could do so it's nicer is require 2 macros IS_A_LT_B and
//    IS_A_EQ_B (or IS_A_GT_B). This can make instantiation in general nicer, at
//    the cost of making the use of strcmp() in these macros a uglier and less
//    performant as we would be calling it twice. Probably the right thing to do
//    is leave it as it is and let the user create a proper function if things
//    get ugly.
//    :3_way_comparison_needed
//
//  - We don't yet have a node removal API.

#define BINARY_TREE_NEW(PREFIX,KEY_TYPE,VALUE_TYPE,CMP_A_TO_B)                                           \
                                                                                                         \
struct PREFIX ## _tree_t {                                                                               \
    mem_pool_t pool;                                                                                     \
                                                                                                         \
    uint32_t num_nodes;                                                                                  \
                                                                                                         \
    struct PREFIX ## _tree_node_t *root;                                                                 \
};                                                                                                       \
                                                                                                         \
struct PREFIX ## _tree_node_t {                                                                          \
    KEY_TYPE key;                                                                                        \
                                                                                                         \
    VALUE_TYPE value;                                                                                    \
                                                                                                         \
    struct PREFIX ## _tree_node_t *right;                                                                \
    struct PREFIX ## _tree_node_t *left;                                                                 \
};                                                                                                       \
                                                                                                         \
void PREFIX ## _tree_destroy (struct PREFIX ## _tree_t *tree)                                            \
{                                                                                                        \
    mem_pool_destroy (&tree->pool);                                                                      \
}                                                                                                        \
                                                                                                         \
struct PREFIX ## _tree_node_t* PREFIX ## _tree_allocate_node (struct PREFIX ## _tree_t *tree)            \
{                                                                                                        \
    struct PREFIX ## _tree_node_t *new_node =                                                            \
        mem_pool_push_struct (&tree->pool, struct PREFIX ## _tree_node_t);                               \
    *new_node = ZERO_INIT(struct PREFIX ## _tree_node_t);                                                \
    return new_node;                                                                                     \
}                                                                                                        \
                                                                                                         \
void PREFIX ## _tree_insert (struct PREFIX ## _tree_t *tree, KEY_TYPE key, VALUE_TYPE value)             \
{                                                                                                        \
    bool key_found = false;                                                                              \
    if (tree->root == NULL) {                                                                            \
        struct PREFIX ## _tree_node_t *new_node = PREFIX ## _tree_allocate_node (tree);                  \
        new_node->key = key;                                                                             \
        new_node->value = value;                                                                         \
                                                                                                         \
        tree->root = new_node;                                                                           \
        tree->num_nodes++;                                                                               \
                                                                                                         \
    } else {                                                                                             \
        struct PREFIX ## _tree_node_t **curr_node = &tree->root;                                         \
        while (!key_found && *curr_node != NULL) {                                                       \
            KEY_TYPE a = key;                                                                            \
            KEY_TYPE b = (*curr_node)->key;                                                              \
            int c = CMP_A_TO_B;                                                                          \
            if (c < 0) {                                                                                 \
                curr_node = &(*curr_node)->left;                                                         \
                                                                                                         \
            } else if (c > 0) {                                                                          \
                curr_node = &(*curr_node)->right;                                                        \
                                                                                                         \
            } else {                                                                                     \
                /* Key already exists. Options of what we could do here:
                   :3_way_comparison_needed

                  - Assert that this will never happen.
                  - Overwrite the existing value with the new one. The problem
                    is if values are pointers, then we could be leaking stuff
                    without knowing?
                  - Do nothing, but somehow let the caller know the key was
                    already there so we didn't insert the value they wanted.

                 I lean more towards the last option.*/                                                  \
                key_found = true;                                                                        \
                break;                                                                                   \
            }                                                                                            \
        }                                                                                                \
                                                                                                         \
        if (!key_found) {                                                                                \
            *curr_node = PREFIX ## _tree_allocate_node (tree);                                           \
            (*curr_node)->key = key;                                                                     \
            (*curr_node)->value = value;                                                                 \
                                                                                                         \
            tree->num_nodes++;                                                                           \
        }                                                                                                \
    }                                                                                                    \
}                                                                                                        \
                                                                                                         \
bool PREFIX ## _tree_lookup (struct PREFIX ## _tree_t *tree,                                             \
                             char *key,                                                                  \
                             struct PREFIX ## _tree_node_t **result)                                     \
{                                                                                                        \
    bool key_found = false;                                                                              \
    struct PREFIX ## _tree_node_t **curr_node = &tree->root;                                             \
    while (*curr_node != NULL) {                                                                         \
        KEY_TYPE a = key;                                                                                \
        KEY_TYPE b = (*curr_node)->key;                                                                  \
        int c = CMP_A_TO_B;                                                                              \
        if (c < 0) {                                                                                     \
            curr_node = &(*curr_node)->left;                                                             \
                                                                                                         \
        } else if (c > 0) {                                                                              \
            curr_node = &(*curr_node)->right;                                                            \
                                                                                                         \
        } else {                                                                                         \
            /*:3_way_comparison_needed*/                                                                 \
            key_found = true;                                                                            \
            break;                                                                                       \
        }                                                                                                \
    }                                                                                                    \
                                                                                                         \
    if (result != NULL) {                                                                                \
        if (key_found) {                                                                                 \
            *result = *curr_node;                                                                        \
        } else {                                                                                         \
            *result = NULL;                                                                              \
        }                                                                                                \
    }                                                                                                    \
                                                                                                         \
    return key_found;                                                                                    \
}

#define BINARY_TREE_FOR(PREFIX,TREE,VARNAME)                                                             \
                                                                                                         \
struct PREFIX ## _tree_node_t *VARNAME = (TREE)->root;                                                   \
for (struct {                                                                                            \
         bool break_needed;                                                                              \
         int stack_idx;                                                                                  \
         struct PREFIX ## _tree_node_t **stack;                                                          \
     } _loop_ctx = {                                                                                     \
         false,                                                                                          \
         0,                                                                                              \
         malloc ((TREE)->num_nodes*sizeof(struct PREFIX ## _tree_node_t))                                \
     };                                                                                                  \
                                                                                                         \
     _loop_ctx.break_needed = false,                                                                     \
     (VARNAME != NULL ?                                                                                  \
        (_loop_ctx.stack[_loop_ctx.stack_idx++] = VARNAME,                                               \
         VARNAME = VARNAME->left,                                                                        \
         0)                                                                                              \
     :                                                                                                   \
        (_loop_ctx.stack_idx == 0 ?                                                                      \
            (_loop_ctx.break_needed = true, 0)                                                           \
        :                                                                                                \
            (VARNAME = _loop_ctx.stack[--_loop_ctx.stack_idx], 0),                                       \
        0)                                                                                               \
     ),                                                                                                  \
     _loop_ctx.break_needed ? free (_loop_ctx.stack), false : true;                                      \
                                                                                                         \
     VARNAME != NULL ?                                                                                   \
         (VARNAME = VARNAME->right, 0) : 0)                                                              \
     if (VARNAME != NULL)

////////////////////////////////////////////////////////////////////////////////
// USE CASE 1
//
// This is how instantiation of this macro looks like. As you can see we have
// removed all the drawbacks of iterating nodes of tree: we have a single line
// for tree instantiation, we don't have a callback (and don't need closures).
//
// This is a HUGE improvement over what could have been the naive approach shown
// in use cases 2 and 3 of binary_tree_v0.c. In fact I like this pattern so much
// that I'm pretty sure I will be using it for linked lists and directory
// iteration too.

BINARY_TREE_NEW(instance_name, char*, int, strcmp(a,b))

int main (int argc, char **argv)
{
    struct instance_name_tree_t tree = {0};

    instance_name_tree_insert (&tree, "zeus", 20);
    instance_name_tree_insert (&tree, "ares", 3);
    instance_name_tree_insert (&tree, "juno", 99);

    BINARY_TREE_FOR (instance_name, &tree, node)
    {
        printf ("%s: %d\n", node->key, node->value);
    }
    printf ("\n");
}

