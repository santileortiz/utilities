/*
 * Copyright (C) 2019 Santiago León O.
 */

void string_tests ()
{
    // Quick test for string functions with the printf syntax. Move this into
    // another file!
    string_t str = {0};
    str_set_printf (&str, "%d, and '%s', plus %c", 120, "FOO", 'o');
    str_cat_printf (&str, ", some appended text %d", 44);
    printf ("%s\n", str_data(&str));
    str_free (&str);
}
