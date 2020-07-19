/*
 * Copiright (C) 2018 Santiago León O.
 */

#include "common.h"

struct tool_t {
    char *name;
};

struct figure_t {
    vect2 *vertices;
};

struct canvas_t {
    mem_pool_t *pool;
    struct tool_t *tools;

    mem_pool_t vertex_pool;
    struct figure_t *figures;
};

struct canvas_t* canvas_new ()
{
    // Bootstrap a memory pool into the heap
    mem_pool_t *pool;
    {
        mem_pool_t bootstrap_pool = {0};
        pool = mem_pool_push_size (&bootstrap_pool, sizeof(mem_pool_t));
        *pool = bootstrap_pool;
    }

    struct canvas_t *cv = mem_pool_push_size (pool, sizeof(struct canvas_t));
    *cv = ZERO_INIT(struct canvas_t);
    cv->pool = pool;

    return cv;
}

void canvas_destroy (struct canvas_t *cv)
{
    mem_pool_destroy (&cv->vertex_pool);
    mem_pool_destroy (cv->pool);
}

void canvas_clear (struct canvas_t *cv)
{
    mem_pool_destroy (&cv->vertex_pool);
    cv->vertex_pool = ZERO_INIT(mem_pool_t);
    cv->figures = NULL;
}
