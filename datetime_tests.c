/*
 * Copyright (C) 2021 Santiago León O.
 */

void invalid_date_read_test (struct test_ctx_t *t, char *date_str)
{
    bool success = true;
    struct date_t res = {0};

    test_push (t, "'%s' (invalid)", date_str);
    bool is_valid = date_read (date_str, &res);
    if (is_valid) {
        str_cat_printf (t->error, "Date should be invalid but isn't.\n");
        success = false;
    }
    test_pop (t, success);
}

void date_read_test_single (struct test_ctx_t *t, char *date_str, struct date_t *expected)
{
    bool success = true;
    struct date_t res = {0};

    test_push (t, "'%s'", date_str);
    bool is_valid = date_read (date_str, &res);
    if (!is_valid) {
        str_cat_printf (t->error, "Valid date marked as invalid.\n");
        success = false;

    } else if (date_cmp (&res, expected) != 0) {
        str_cat_printf (t->error, "Wrong parsing of '%s':\n", date_str);
        str_date_internal (t->error, &res, expected);
        success = false;
    }
    test_pop (t, success);
}

void date_read_test_all_offsets (struct test_ctx_t *t, char *date_str, struct date_t *expected)
{
    date_read_test_single (t, date_str, expected);

    int num_replacements;
    mem_pool_t pool = {0};
    char *date_str_l = cstr_dupreplace (&pool, date_str, "Z", "+6:5", &num_replacements);
    if (num_replacements > 0) {
        struct date_t expected_l = *expected;

        // Positive UTC offset
        expected_l.time_offset_hour = 6;
        expected_l.time_offset_minute = 5;
        date_read_test_single (t, date_str_l, &expected_l);
        date_str_l = cstr_dupreplace (&pool, date_str, "Z", "+06:05", &num_replacements);
        date_read_test_single (t, date_str_l, &expected_l);

        // Negative UTC offset
        expected_l.time_offset_hour = -6;
        date_str_l = cstr_dupreplace (&pool, date_str, "Z", "-6:5", &num_replacements);
        date_read_test_single (t, date_str_l, &expected_l);
        date_str_l = cstr_dupreplace (&pool, date_str, "Z", "-06:05", &num_replacements);
        date_read_test_single (t, date_str_l, &expected_l);

        // Unknown UTC offset
        expected_l.is_set_time_offset = false;
        date_str_l = cstr_rstrip(cstr_dupreplace (&pool, date_str, "Z", "", &num_replacements));
        date_read_test_single (t, date_str_l, &expected_l);
        date_str_l = cstr_dupreplace (&pool, date_str, "Z", "-00:00", &num_replacements);
        date_read_test_single (t, date_str_l, &expected_l);

    }
    mem_pool_destroy (&pool);
}

void date_read_test (struct test_ctx_t *t, char *date_str, struct date_t *expected)
{
    date_read_test_all_offsets (t, date_str, expected);

    char *buff = malloc (strlen(date_str) + 1);
    int num_replacements = cstr_replace_char_buff (date_str, ' ', 'T', buff);
    if (num_replacements > 0) date_read_test_all_offsets (t, buff, expected);

    num_replacements = cstr_replace_char_buff (date_str, ' ', 't', buff);
    if (num_replacements > 0) date_read_test_all_offsets (t, buff, expected);

    free (buff);
}

void datetime_tests (struct test_ctx_t *t)
{
    test_push (t, "Date and Time");

    {
        test_push (t, "Date read");
        struct date_t _expected = {0};
        struct date_t *expected = &_expected;

        // NOTE(sleon): Use seconds fraction of 0.125 because it's exactly
        // representable as floating point value, this means it can be safely
        // compared with equality and we don't depend on the rounding mode.

        date_set (expected, 1900, 8, 7, 2, 3, 4, 0.125, true, 0, 0);
        date_read_test (t, "1900-8-7 2:3:4.125Z", expected);
        date_read_test (t, "1900-08-07 02:03:04.125Z", expected);

        date_set (expected, 1900, 1, 1, 20, 30, -1, 0.0, true, 0, 0);
        date_read_test (t, "1900-01-01 20:30Z", expected);
        date_set (expected, 1900, 1, 1, 20, -1, -1, 0.0, true, 0, 0);
        date_read_test (t, "1900-01-01 20Z", expected);
        date_set (expected, 1900, 1, 1, -1, -1, -1, 0.0, true, 0, 0);
        date_read_test (t, "1900-01-01 Z", expected);
        date_set (expected, 1900, 1, -1, -1, -1, -1, 0.0, true, 0, 0);
        date_read_test (t, "1900-01 Z", expected);
        date_set (expected, 1900, -1, -1, -1, -1, -1, 0.0, true, 0, 0);
        date_read_test (t, "1900 Z", expected);

        parent_test_pop (t);
    }

    {
        test_push (t, "Recurrent event tests");

        char res[DATE_TIME_LEN];
        struct recurrent_event_t re = {0};

        {
            bool success = true;
            test_push (t, "Every 7th day");
            set_recurrent_event (&re, 7, D_DAY, NULL, "1908-01-4");
            //set_recurrent_event_compact (&re, "_-_-[7]", "1908-01-4");
            compute_next_occurence (&re, NULL, res);

            char *expected = "1908-01-11";
            if (strcmp (res, expected) != 0) {
                str_cat_printf (t->error, "Incorrect next occurence got %s, expected %s\n", res, expected);
                success = false;
            }
            test_pop (t, success);
        }

        parent_test_pop (t);
    }

    parent_test_pop (t);
}
