/*
 * Copiright (C) 2019 Santiago León O.
 */

// The idea of this is to discuss the creation of an API that makes a good
// abstraction for linked lists. There are several goals of this API and I
// haven't found one that meets them all, which is why, so far I just write
// linked lists manually every time I need one. Main goals are:
//
//  * Flexibility of linked list implementation. Linked lists can have very
//    different performance profiles depending on implementation details. I
//    don't want to force the user into a model I choose to be right. Instead I
//    want to provide tools (functions or macros) that allow easy implementation
//    and use of different kinds of linked lists.
//
//    Example: Appending is usually considered to be O(n), but keeping a pointer
//    to the end makes it O(1), if the caller stores this value we can do O(1).
//    But, we don't want all linked lists to contain the end pointer, if the
//    user is appending at the beginning it's not necessary.
//
//  * Non-abstractedness. Don't hide implementation details that make it hard to
//    know the performance costs of the implemented linked list.
//
//  * We don't care about managing memory of nodes, we let the caller do that.
//    There is no node type, instead the caller can make a llinked list out of
//    any struct and then instantiate our API for that type. This plays niceley
//    with memory allocation mechanisms like memory pools.
//
//  * Because we don't have a node type, we also don't care about the type of
//    data stored in the list. Glib for example always stores pointers and makes
//    it very ugly to create a list that is just storing integers. We could
//    provide default instantiations for basic types like int.
//
//  * Easy to learn API. We want to have as few functions as possible and
//    have a consistent naming scheme that makes them easy to remember.
//
// One of the main use cases this API will be used for is to create free lists
// when we remove nodes from a linked list where nodes have been allocated
// inside a pool.

struct my_struct_type_t {
    int value;

    struct my_struct_type_t *next;
};


// How different APIs would be used to create a list by appending nodes at the
// end of a list.
void create_list_at_the_end_examples () {
    struct my_struct_type_t *start=NULL, *end=NULL;
    while (...) {
        if (start==NULL) {
            start = new_node;
            end = new_node;
        } else {
            type_list_ia1 (end, new_node);
            end = new_node;
        }
    }

    struct bla_t *start=NULL, *end=NULL;
    while (...) {
        type_list_ia1 (end, new_node);
        end = new_node;

        if (start == NULL) {
            start = new_node;
        }
    }

    struct bla_t *start=NULL, *end=NULL;
    while (...) {
        // This is mandatory if new_node hasn't been zero initialized or is
        // being reused (which is the case when creating a free list). Should
        // this be inside the function call? The problem is then we can't use
        // the same function call for list concatenation, or insertion in the
        // middle of the list.
        new_node = NULL;

        type_list_ia2 (&start, end, new_node);
        end = new_node;
    }
}

void iterate_list_examples () {
    while (
}


