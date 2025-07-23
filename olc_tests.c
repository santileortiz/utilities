#include "lib/olc.c"
#include "olc_utility.h"

void olc_tests (struct test_ctx_t *t)
{
    test_push (t, "OLC (Plus Codes)");

    string_t str = {};

    str_cat_plus_code (&str, 4.946974903070454, -73.96281969128495);
    test_str (t, "encoding", str_data(&str), "67P8W2WP+QV");

    str_free(&str);

    test_pop_parent (t);
}
