/*
 * Copiright (C) 2019 Santiago León O.
 */

struct my_linked_list_t {
    int id;

    string_t name;

    struct my_linked_list_t *next;
};

bool my_linked_list_check (struct my_linked_list_t *list, int start_id, int end_id, string_t *error)
{
    bool success = true;

    int expected_num_elements;
    if (start_id < end_id) {
        expected_num_elements = end_id - start_id + 1;
    } else {
        expected_num_elements = start_id - end_id + 1;
    }

    // Check the number of elements
    {
        int node_cnt = 0;
        struct my_linked_list_t *curr_node = list;
        while (curr_node != NULL) {
            node_cnt++;
            curr_node = curr_node->next;
        }
        if (node_cnt != expected_num_elements) {
            str_cat_printf (error, "List has %d elements, expected %d.\n", node_cnt, expected_num_elements);
            // TODO: Print the ids of the first and last extra elements
            success = false;
        }
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

        if (success) {
            if (linked_list_a != NULL) {
                str_cat_printf (t->error, "Expected linked_list_a to be empty.\n");
                success = false;
            }

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

        if (success) {
            if (linked_list_b != NULL) {
                str_cat_printf (t->error, "Expected linked_list_b to be empty.\n");
                success = false;
            }

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

        success = my_linked_list_check (linked_list_c, 0, num_elements-1, t->error); 

        test_pop (t, success);
    }

    // TODO: Check that LINKED_LIST_REMOVE() actually removes the passed node

    // TODO: Check appending an item from an other list
    // TODO: Check pushing an item from an other list
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

        success = my_linked_list_check (linked_list_c, 0, num_elements-1, t->error); 
        success = my_linked_list_check (linked_list_a, start_id+num_elements-1, start_id, t->error); 

        test_pop (t, success);
    }

    mem_pool_destroy (&pool);

    parent_test_pop (t);
}
