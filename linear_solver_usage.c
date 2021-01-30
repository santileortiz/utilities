/*
 * Copyright (C) 2020 Santiago León O.
 */

#define _GNU_SOURCE // Used to enable strcasestr()
#define _XOPEN_SOURCE 700 // Required for strptime()
#include "common.h"

#include "scanner.c"
#include "binary_tree.c"
#include "linear_solver.c"

int main (int argc, char **argv)
{
    struct linear_system_t system = {0};

    solver_expr_equals_zero (&system, "x1 - x2 + x3 - c1");
    solver_expr_equals_zero (&system, "x1 + x2 - x3 - c2");
    solver_expr_equals_zero (&system, "x1 + x2 + x3 + c3");

    solver_symbol_assign (&system, "c1", 8);
    solver_symbol_assign (&system, "c2", -11);
    solver_symbol_assign (&system, "c3", -3);

    string_t error = {0};
    bool success = solver_solve (&system, &error);

    printf ("Symbols:\n");
    BINARY_TREE_FOR (name_to_symbol_definition, &system.name_to_symbol_definition, curr_node) {
        struct symbol_definition_t *symbol_definition = curr_node->value;
        if (symbol_definition->state == SYMBOL_ASSIGNED) {
            printf ("%s = %.2f\n", str_data(&symbol_definition->name), symbol_definition->value);
        } else {
            printf ("%s = ?\n", str_data(&symbol_definition->name));
        }
    }
    printf ("\n");

    int num_unassigned_symbols = 0;
    {
        printf ("Assigned:\n");
        BINARY_TREE_FOR (name_to_symbol_definition, &system.name_to_symbol_definition, curr_node) {
            struct symbol_definition_t *symbol_definition = curr_node->value;
            if (symbol_definition->state == SYMBOL_ASSIGNED) {
                printf ("%s = %.2f\n", str_data(&symbol_definition->name), symbol_definition->value);
            } else {
                num_unassigned_symbols++;
            }
        }
        printf ("\n");
    }

    {
        printf ("Solved:\n");
        BINARY_TREE_FOR (name_to_symbol_definition, &system.name_to_symbol_definition, curr_node) {
            struct symbol_definition_t *symbol_definition = curr_node->value;
            if (symbol_definition->state == SYMBOL_SOLVED) {
                printf ("%s = %.2f\n", str_data(&symbol_definition->name), symbol_definition->value);
            }
        }
    }

    printf ("\n");
    printf ("Total symbols: %d\n", system_num_symbols (&system));
    printf ("Symbols to solve: %d\n", num_unassigned_symbols);
    printf ("Equations: %d\n", system_num_equations (&system));

    if (!success) {
        printf ("\n");
        printf ("%s", str_data(&error));
    }

    solver_destroy (&system);

    return 0;
}
