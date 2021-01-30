/*
 * Copyright (C) 2018 Santiago León O.
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

struct canvas_t* canvas_new (mem_pool_t *pool)
{
    struct canvas_t *cv = mem_pool_push_size (pool, sizeof(struct canvas_t));
    *cv = ZERO_INIT(struct canvas_t);
    cv->pool = pool;

    mem_pool_add_child (pool, &cv->vertex_pool);

    return cv;
}

void canvas_clear (struct canvas_t *cv)
{
    mem_pool_destroy (&cv->vertex_pool);
    cv->vertex_pool = ZERO_INIT(mem_pool_t);
    cv->figures = NULL;
}
