/*
 * Copiright (C) 2019 Santiago León O.
 */

// Maybe we want people to do this manually? so that they know exactly what it
// means storage wise to have a linked list?. It's also possible that you want
// to save up on the number of pointers and the end will be computed before
// appending.
#define LINKED_LIST_DECLARE(type,head_name) \
    type *head_name;                        \
    type *head_name ## _end;

#define LINKED_LIST_APPEND(type,head_name,node)              \
{                                                            \
    if (head_name ## _end == NULL) {                         \
        head_name = node;                                    \
    } else {                                                 \
        head_name ## _end->next = node;                      \
    }                                                        \
    head_name ## _end = node;                                \
}

#define LINKED_LIST_APPEND_NEW(type,head_name,pool,new_node) \
type *new_node;                                              \
{                                                            \
    new_node = mem_pool_push_struct(pool,type);              \
    *new_node = ZERO_INIT(type);                             \
                                                             \
    LINKED_LIST_APPEND(type,head_name,new_node)              \
}

#define LINKED_LIST_PUSH(type,head_name,node)                \
{                                                            \
    node->next = head_name;                                  \
    head_name = node;                                        \
}

#define LINKED_LIST_PUSH_NEW(type,head_name,pool,new_node)   \
type *new_node;                                              \
{                                                            \
    new_node = mem_pool_push_struct(pool,type);              \
    *new_node = ZERO_INIT(type);                             \
                                                             \
    LINKED_LIST_PUSH(type,head_name,new_node)                \
}

// TODO: Implement LINKED_LIST_FOR like BINARY_TREE_FOR
