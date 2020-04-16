/*
 * Copiright (C) 2019 Santiago León O.
 */

struct my_linked_list_t {
    int id;

    string_t name;

    struct my_linked_list_t *next;
};

bool check_list_size (struct my_linked_list_t *list, int expected_num_elements, int *node_count, string_t *error)
{
    bool success = true;

    int node_cnt = 0;
    struct my_linked_list_t *curr_node = list;
    while (curr_node != NULL) {
        node_cnt++;
        curr_node = curr_node->next;
    }
    if (node_cnt != expected_num_elements) {
        str_cat_printf (error, "List has %d elements, expected %d.\n", node_cnt, expected_num_elements);
        success = false;
    }

    if (node_count != NULL) {
        *node_count = node_cnt;
    }

    return success;
}

bool my_linked_list_check (struct my_linked_list_t *list, int start_id, int end_id, string_t *error)
{
    bool success = true;

    bool ascending;
    int expected_num_elements;
    if (start_id < end_id) {
        expected_num_elements = end_id - start_id + 1;
        ascending = true;
    } else {
        expected_num_elements = start_id - end_id + 1;
        ascending = false;
    }

    int node_cnt;
    success = check_list_size (list, expected_num_elements, &node_cnt, error);
    if (!success) {
        // Print an informative error message with details about the expected
        // and actual list.

        int max_elements_to_print = 8;
        str_cat_printf (error, "[");
        if (expected_num_elements > max_elements_to_print) {
            int elements_per_edge = max_elements_to_print/2;

            int i;
            for (i=0; i < elements_per_edge; i++) {
                int id = ascending ? start_id + i : start_id - i;
                str_cat_printf (error, "%d, ", id);
            }

            str_cat_printf (error, "..., ");

            for (i=elements_per_edge-1; i > 0; i--) {
                int id = ascending ? end_id - i : end_id + i;
                str_cat_printf (error, "%d, ", id);
            }
            str_cat_printf (error, "%d", end_id + i);

        } else {
            int i;
            for (i=0; i < expected_num_elements-1; i++) {
                str_cat_printf (error, "%d, ", start_id + i);
            }
            str_cat_printf (error, "%d", start_id + i);
        }

        str_cat_printf (error, "] != [");

        int node_idx = 0;
        struct my_linked_list_t *curr_node = list;
        while (curr_node->next != NULL) {
            // NOTE: The order of these conditions is important.
            if (node_idx < max_elements_to_print/2) {
                str_cat_printf (error, "%d, ", curr_node->id);

            } else if (node_idx > node_cnt - max_elements_to_print/2 - 1) {
                str_cat_printf (error, "%d, ", curr_node->id);

            } else if (node_idx == expected_num_elements - 1) {
                str_cat_printf (error, "(%d), ", curr_node->id);

            } else if (node_idx == expected_num_elements - 2 || node_idx == expected_num_elements) {
                str_cat_printf (error, "%d, ", curr_node->id);

            } else if (node_idx == max_elements_to_print/2) {
                str_cat_printf (error, "..., ");

            } else if (node_cnt > expected_num_elements &&
                       node_idx == node_cnt - max_elements_to_print/2 - 1) {
                str_cat_printf (error, "..., ");
            }

            node_idx++;
            curr_node = curr_node->next;
        }
        str_cat_printf (error, "%d]\n", curr_node->id);

        success = false;
    }

    // Check the ids of elements
    {
        bool ascending = start_id < end_id;
        int expected_id = start_id;
        int node_cnt = 0;
        struct my_linked_list_t *curr_node = list;
        while (curr_node != NULL && node_cnt < expected_num_elements) {
            if (expected_id != curr_node->id) {
                str_cat_printf (error, "Expected id=%d but got id=%d.\n", expected_id, curr_node->id);
                success = false;
                break;
            }

            if (ascending) {
                expected_id++;
            } else {
                expected_id--;
            }
            node_cnt++;
            curr_node = curr_node->next;
        }
    }

    return success;
}

bool my_linked_list_check_skip (struct my_linked_list_t *list, int start_id, int end_id, int id_to_skip,
                                string_t *error)
{
    bool success = true;

    int expected_num_elements;
    if (start_id < end_id) {
        expected_num_elements = end_id - start_id;
    } else {
        expected_num_elements = start_id - end_id;
    }

    success = check_list_size (list, expected_num_elements, NULL, error);

    // Check the ids of elements
    {
        bool ascending = start_id < end_id;
        int expected_id = start_id;
        int node_cnt = 0;
        struct my_linked_list_t *curr_node = list;
        while (curr_node != NULL && node_cnt < expected_num_elements) {
            if (expected_id != id_to_skip) {
                if (expected_id != curr_node->id) {
                    str_cat_printf (error, "Expected id=%d but got id=%d.\n", expected_id, curr_node->id);
                    success = false;
                    break;
                }

            } else {
                if (ascending) {
                    expected_id++;
                } else {
                    expected_id--;
                }
            }

            if (ascending) {
                expected_id++;
            } else {
                expected_id--;
            }

            node_cnt++;
            curr_node = curr_node->next;
        }
    }

    return success;
}

void linked_list_tests (struct test_ctx_t *t)
{
    test_push (t, "Linked List");

    bool success = true;
    mem_pool_t pool = {0};
    int num_elements = 100;

    struct my_linked_list_t *linked_list_a = NULL;
    {
        test_push (t, "Linked list push");

        CRASH_TEST_AND_RUN(success, t->error,
            for (int id=0; id<num_elements; id++) {
                LINKED_LIST_PUSH_NEW (&pool, struct my_linked_list_t, linked_list_a, my_node);
                my_node->id = id;

                str_set_printf (&my_node->name, "Node @%d", id);
            }
        );

        test_pop (t, success);
    }

    if (success) {
        test_push (t, "Linked list iteration");

        int node_id_sum = 0;
        CRASH_TEST_AND_RUN(success, t->error,
            struct my_linked_list_t *curr_node = linked_list_a;
            while (curr_node != NULL) {
                node_id_sum += curr_node->id;

                curr_node = curr_node->next;
            }
        );

        if (success) {
            int expected_id_sum = (num_elements*(num_elements-1))/2;
            if (node_id_sum != expected_id_sum) {
                str_cat_printf (t->error, "Computed ID sum is %d, expected %d.\n", node_id_sum, expected_id_sum);
                success = false;
            }
        }

        if (success) {
            success = my_linked_list_check (linked_list_a, num_elements-1, 0, t->error); 
        }

        test_pop (t, success);
    }

    struct my_linked_list_t *linked_list_b = NULL;

    // POP from linked_list_a -> PUSH into linked_list_b
    if (success) {
        test_push (t, "Linked list pop");

        int node_cnt = 0;

        CRASH_TEST_AND_RUN(success, t->error,
            while (linked_list_a != NULL && node_cnt < num_elements) {
                struct my_linked_list_t *popped_node = LINKED_LIST_POP (linked_list_a);

                LINKED_LIST_PUSH (linked_list_b, popped_node);
                
                node_cnt++;
            }
        );

        if (success && linked_list_a != NULL) {
            str_cat_printf (t->error, "Expected linked_list_a to be empty.\n");
            success = false;
        }

        if (success) {
            success = my_linked_list_check (linked_list_b, 0, num_elements-1, t->error); 
        }

        test_pop (t, success);
    }

    struct my_linked_list_t *linked_list_c = NULL;
    struct my_linked_list_t *linked_list_c_end = NULL;

    // POP from linked_list_b -> APPEND into linked_list_c
    if (success) {
        test_push (t, "Linked list append");

        int node_cnt = 0;

        CRASH_TEST_AND_RUN(success, t->error,
            while (linked_list_b != NULL && node_cnt < num_elements) {
                struct my_linked_list_t *popped_node = LINKED_LIST_POP (linked_list_b);

                LINKED_LIST_APPEND (linked_list_c, popped_node);
                
                node_cnt++;
            }
        );

        if (success && linked_list_b != NULL) {
            str_cat_printf (t->error, "Expected linked_list_b to be empty.\n");
            success = false;
        }

        if (success) {
            success = my_linked_list_check (linked_list_c, 0, num_elements-1, t->error); 
        }

        test_pop (t, success);
    }

    if (success) {
        test_push (t, "Non existent item remove");

        struct my_linked_list_t node_not_in_list = {0};

        CRASH_TEST_AND_RUN(success, t->error,
            LINKED_LIST_REMOVE (struct my_linked_list_t, linked_list_c, &node_not_in_list);
        );

        if (success) {
            success = my_linked_list_check (linked_list_c, 0, num_elements-1, t->error);
        }

        test_pop (t, success);
    }

    struct my_linked_list_t *a_node = NULL;
    int start_id = 200;
    for (int i=0; i<num_elements; i++) {
        LINKED_LIST_PUSH_NEW (&pool, struct my_linked_list_t, linked_list_a, new_node);
        if (i == num_elements/2) {
            a_node = new_node;
        }
        new_node->id = i+start_id;
    }

    if (success) {
        test_push (t, "Remove item in other list");

        CRASH_TEST_AND_RUN(success, t->error,
            LINKED_LIST_REMOVE (struct my_linked_list_t, linked_list_c, a_node);
        );

        if (success) {
            success = my_linked_list_check (linked_list_c, 0, num_elements-1, t->error);
        }

        if (success) {
            success = my_linked_list_check (linked_list_a, start_id+num_elements-1, start_id, t->error);
        }

        test_pop (t, success);
    }

    // NOTE: Pushing or appending elements that are already in other list is
    // problematic. There is no way we can know the node belongs somewhere else
    // and we wouldn't know the head of the other list, which makes it
    // impossible to remove it from the other list. What most likely will happen
    // is it will join both lists. Maybe this behavior is not expected and what
    // we would really like is to assert if the user pushes an element where
    // next is not NULL, but still, it could be the end of another list. I'm
    // thinking if the user is getting a lot of these problems, then they should
    // better use a doubly linked list.

    if (success) {
        test_push (t, "Remove first element");

        CRASH_TEST_AND_RUN(success, t->error,
            LINKED_LIST_REMOVE (struct my_linked_list_t, linked_list_c, linked_list_c);
        );

        if (success) {
            success = my_linked_list_check (linked_list_c, 1, num_elements-1, t->error);
        }

        test_pop (t, success);
    }

    if (success) {
        test_push (t, "Remove last element");

        CRASH_TEST_AND_RUN(success, t->error,
            LINKED_LIST_REMOVE (struct my_linked_list_t, linked_list_c, linked_list_c_end);
        );

        if (success && linked_list_c_end->id != num_elements-2) {
            str_cat_printf (t->error, "End pointer of linked_list_c has id=%d, expected %d.\n",
                            linked_list_c_end->id, num_elements-2);
            success = false;
        }

        if (success) {
            success = my_linked_list_check (linked_list_c, 1, num_elements-2, t->error);
        }

        test_pop (t, success);
    }

    if (success) {
        test_push (t, "Remove middle element");

        struct my_linked_list_t *middle_element = linked_list_c;
        while (middle_element->id != num_elements/2) {
            middle_element = middle_element->next;
        }

        CRASH_TEST_AND_RUN(success, t->error,
            LINKED_LIST_REMOVE (struct my_linked_list_t, linked_list_c, middle_element);
        );

        if (success) {
            success = my_linked_list_check_skip (linked_list_c, 1, num_elements-2, middle_element->id, t->error);
        }

        test_pop (t, success);
    }

    if (success) {
        test_push (t, "Remove until end");

        CRASH_TEST_AND_RUN(success, t->error,
            while (linked_list_c != NULL) {
                LINKED_LIST_REMOVE (struct my_linked_list_t, linked_list_c, linked_list_c);
            }
        );

        if (success && linked_list_c_end != NULL) {
            str_cat_printf (t->error, "End pointer of linked_list_c has id=%d, expected NULL pointer.\n",
                            linked_list_c_end->id);
            success = false;
        }

        test_pop (t, success);
    }

    mem_pool_destroy (&pool);

    parent_test_pop (t);
}
