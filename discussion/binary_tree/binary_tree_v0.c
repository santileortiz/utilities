/*
 * Copiright (C) 2019 Santiago León O.
 */

////////////////
// API V0
//
// This is the first binary tree I implemented. It had specific key and value
// types. The obvious next stabe would be to make it instantiatable.
//
// The main problem with turning this code into a macro is the
// binary_tree_foreach_cb_t type used in binary_tree_foreach(). A pattern we
// normally use to make maintaíning callback declarations is creating a macro
// like BINARY_TREE_FOREACH_CB(), this way we only have one place where argument
// declaration, names and orderig are defined. Making possible to change them in
// the future without breaking code. The problem is C does not allow defining a
// macro inside a macro. Making impossible to instantiate this macro for each
// kind of binary tree struct.
//
// We can leave this function out of the generic binary tree, but iteration over
// elements is so common that I don't like that approach. We can also include it
// and make the user declare the binary_tree_foreach_cb_t type every time they
// need node iteration. I also don't like this approach as it adds mental
// overhead when instantiating this type. Finally the better approach would be
// to have a generic iteration macro that works for any binary tree
// instantiation.

struct binary_tree_t {
    mem_pool_t pool;

    uint32_t num_nodes;

    struct binary_tree_node_t *root;
};

// Leftmost node will be the smallest.
struct binary_tree_node_t {
    char *key;

    int value;

    struct binary_tree_node_t *right;
    struct binary_tree_node_t *left;
};

void binary_tree_destroy (struct binary_tree_t *tree)
{
    mem_pool_destroy (&tree->pool);
}

struct binary_tree_node_t* binary_tree_allocate_node (struct binary_tree_t *tree)
{
    // TODO: When we add removal of nodes, this should allocate them from a free
    // list of nodes.
    struct binary_tree_node_t *new_node =
        mem_pool_push_struct (&tree->pool, struct binary_tree_node_t);
    *new_node = ZERO_INIT(struct binary_tree_node_t);
    return new_node;
}

void binary_tree_insert (struct binary_tree_t *tree, char *key, int value)
{
    if (tree->root == NULL) {
        struct binary_tree_node_t *new_node = binary_tree_allocate_node (tree);
        new_node->key = key;
        new_node->value = value;

        tree->root = new_node;
        tree->num_nodes++;

    } else {
        bool key_found = false;
        struct binary_tree_node_t **curr_node = &tree->root;
        while (*curr_node != NULL) {
            int c = strcmp (key, (*curr_node)->key);
            if (c < 0) {
                curr_node = &(*curr_node)->left;

            } else if (c > 0) {
                curr_node = &(*curr_node)->right;

            } else {
                // Key already exists. Options of what we could do here:
                //
                //  - Assert that this will never happen.
                //  - Overwrite the existing value with the new one. The problem
                //    is if values are pointers in the future, then we could be
                //    leaking stuff without knowing?
                //  - Do nothing, but somehow let the caller know the key was
                //    already there so we didn't insert the value they wanted.
                //
                // I lean more towards the last option.
                key_found = true;
                break;
            }
        }

        if (!key_found) {
            *curr_node = binary_tree_allocate_node (tree);
            (*curr_node)->key = key;
            (*curr_node)->value = value;

            tree->num_nodes++;
        }

        // TODO: Rebalance the tree.
    }
}

bool binary_tree_lookup (struct binary_tree_t *tree, char *key, struct binary_tree_node_t **result)
{
    bool key_found = false;
    struct binary_tree_node_t **curr_node = &tree->root;
    while (*curr_node != NULL) {
        int c = strcmp (key, (*curr_node)->key);
        if (c < 0) {
            curr_node = &(*curr_node)->left;

        } else if (c > 0) {
            curr_node = &(*curr_node)->right;

        } else {
            key_found = true;
            break;
        }
    }

    if (result != NULL) {
        if (key_found) {
            *result = *curr_node;
        } else {
            *result = NULL;
        }
    }

    return key_found;
}

#define BINARY_TREE_FOREACH_CB(name) void name(struct binary_tree_node_t *node, void *data)
typedef BINARY_TREE_FOREACH_CB(binary_tree_foreach_cb_t);

void binary_tree_foreach (struct binary_tree_t *tree, binary_tree_foreach_cb_t *cb, void *data)
{
    if (tree->num_nodes < 1) return;

    mem_pool_t pool = {0};

    // TODO: Maybe get a real stack here? seems wasteful to create an array that
    // could hold all nodes. When we implement balancing, we can make this array
    // of size log(num_nodes), just need to be sure of an upper bound.
    struct binary_tree_node_t **stack =
        mem_pool_push_array (&pool, tree->num_nodes, struct binary_tree_node_t);

    int stack_idx = 0;
    struct binary_tree_node_t *curr_node = tree->root;
    while (true) {
        if (curr_node != NULL) {
            stack[stack_idx++] = curr_node;
            curr_node = curr_node->left;

        } else {
            if (stack_idx == 0) break;

            curr_node = stack[--stack_idx];
            cb (curr_node, data);

            curr_node = curr_node->right;
        }
    }

    mem_pool_destroy (&pool);
}

////////////////////////////////////////////////////////////////////////////////
// USE CASE 1
//
// This would be the ideal usage of these functions.

BINARY_TREE_FOREACH_CB(print_keys_cb)
{
    printf ("%s: %d\n", node->key, node->value);
}

int main (int argc, char **argv)
{
    struct binary_tree_t tree = {0};

    binary_tree_insert (&tree, "zeus", 20);
    binary_tree_insert (&tree, "ares", 3);
    binary_tree_insert (&tree, "juno", 99);

    binary_tree_foreach (&tree, print_keys_cb, NULL);
    printf ("\n");
}

////////////////////////////////////////////////////////////////////////////////
// USE CASE 2
//
// This show how it would look like if we were to turn all functions into
// macros in the naive way.
//
// I don't like the callback paradigm because it makes the user create a closure
// and declare a function every time they want to iterate ofer items. That means
// coming up with the callback name, the closure name, the callback declaration
// and the closure decration. Much more than what I like, and it also adds
// overhead each time we need a variable inside the loop, it needs to be added
// in the loop, in the loop declaration and in the closure population.

// Main tree declaration that doesn't include instance_name_tree_foreach()
// because we can't declare the callback type macros we would like to have for
// easy maintainership.
BINARY_TREE_NEW(instance_name, char*, int, strcmp(a,b))

// Optional instantiation statements, required every time the user wants to
// iterate over elements
#define INSTANCE_NAME_FOREACH_CB(name) void name(struct str_int_tree_node_t *node, void *data)
typedef INSTANCE_NAME_FOREACH_CB(instance_name_tree_foreach_cb_t);
BINARY_TREE_FOR(instance_name)

INSTANCE_NAME_FOREACH_CB(print_keys_cb)
{
    printf ("%s: %d\n", node->key, node->value);
}

int main (int argc, char **argv)
{
    struct instance_name_tree_t tree = {0};

    instance_name_tree_insert (&tree, "zeus", 20);
    instance_name_tree_insert (&tree, "ares", 3);
    instance_name_tree_insert (&tree, "juno", 99);

    instance_name_tree_foreach (&tree, print_keys_cb, NULL);
    printf ("\n");
}

////////////////////////////////////////////////////////////////////////////////
// USE CASE 3
//
// This is another way of doing the same thing as in the use case 2 but
// including instantiation of instance_name_tree_foreach() in the tree
// instantiation macro. It saves one line at instantiation time at the cost of
// making the user declare the callback type every time a tree is instantiated.
// This could be better because we ask for all the mental overhead upfront at
// instantiation time and not later when they find out they need to iterate
// nodes.
//
// If I recall correctly I think this macros would throw compiler warnings
// becaúse we are using instance_name_tree_foreach_cb_t inside BINARY_TREE_NEW
// without declaring it anywhere else. This was also an issue in use case 2. I
// don't remember because I wrote the code and soon discovered it was too
// coumbersome an moved on to developing the binary_tree_v1 API.

#define INSTANCE_NAME_FOREACH_CB(name) void name(struct str_int_tree_node_t *node, void *data)
typedef INSTANCE_NAME_FOREACH_CB(instance_name_tree_foreach_cb_t);
BINARY_TREE_NEW(instance_name, char*, int, strcmp(a,b))

INSTANCE_NAME_FOREACH_CB(print_keys_cb)
{
    printf ("%s: %d\n", node->key, node->value);
}

int main (int argc, char **argv)
{
    struct instance_name_tree_t tree = {0};

    instance_name_tree_insert (&tree, "zeus", 20);
    instance_name_tree_insert (&tree, "ares", 3);
    instance_name_tree_insert (&tree, "juno", 99);

    instance_name_tree_foreach (&tree, print_keys_cb, NULL);
    printf ("\n");
}
