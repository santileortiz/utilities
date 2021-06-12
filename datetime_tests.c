/*
 * Copyright (C) 2021 Santiago León O.
 */

void different_date_compare_test (struct test_ctx_t *t, char *test_name, struct date_t *d1, struct date_t *d2)
{
    bool success = true;
    test_push (t, "%s", test_name);
    if (date_cmp (d1, d2) == 0) {
        str_cat_printf (t->error, "Distinct dates marked as equal\n");
        str_date_internal (t->error, d1, d2);
        success = false;
    }
    test_pop (t, success);
}

void date_compare_test (struct test_ctx_t *t, char *test_name, struct date_t *d1, struct date_t *d2)
{
    bool success = true;
    test_push (t, "%s", test_name);
    if (date_cmp (d1, d2) != 0) {
        str_cat_printf (t->error, "Equal dates marked as different\n");
        str_date_internal (t->error, d1, d2);
        success = false;
    }
    test_pop (t, success);
}

void invalid_date_read_test (struct test_ctx_t *t, char *date_str)
{
    bool success = true;
    struct date_t res = {0};

    test_push (t, "'%s' (invalid)", date_str);
    bool is_valid = date_read (date_str, &res, NULL);
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
    string_t message = {0};
    bool is_valid = date_read (date_str, &res, &message);
    if (!is_valid) {
        str_cat_printf (t->error, "Valid date marked as invalid:\n  '%s'\n", str_data(&message));
        success = false;

    } else if (date_cmp (&res, expected) != 0) {
        str_cat_printf (t->error, "Wrong parsing of '%s':\n", date_str);
        str_date_internal (t->error, &res, expected);
        success = false;
    }
    str_free (&message);
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

void string_test (struct test_ctx_t *t, char *test_name, char *result, char *expected)
{
    bool success = true;
    test_push (t, "%s (%s)", expected, test_name);
    if (strcmp (result, expected) != 0) {
        str_cat_printf (t->error, "Failed string comparison got '%s', expected '%s'\n", result, expected);
        success = false;
    }
    test_pop (t, success);
}

void int_test (struct test_ctx_t *t, char *test_name, int result, int expected)
{
    bool success = true;
    test_push (t, "%s", test_name);
    if (result != expected) {
        str_cat_printf (t->error, "Failed int comparison got %d, expected %d\n", result, expected);
        success = false;
    }
    test_pop (t, success);
}

void date_write_rfc3339_test (struct test_ctx_t *t, struct date_t *date, char *expected)
{
    test_push (t, "%s", expected);

    string_t test_date = {0};
    str_set (&test_date, expected);
    size_t base_end = str_len(&test_date);

    char buff[date_max_len[D_SECOND]];

    date->is_set_time_offset = true;
    date->time_offset_hour = 0;
    date->time_offset_minute = 0;
    str_put_c (&test_date, base_end, "Z");
    date_write_rfc3339 (date, buff);
    string_test (t, "UTC offset", buff, str_data(&test_date));

    date->time_offset_hour = 6;
    date->time_offset_minute = 30;
    str_put_c (&test_date, base_end, "+06:30");
    date_write_rfc3339 (date, buff);
    string_test (t, "Positive UTC offset", buff, str_data(&test_date));

    date->time_offset_hour = -6;
    str_put_c (&test_date, base_end, "-06:30");
    date_write_rfc3339 (date, buff);
    string_test (t, "Negative UTC offset", buff, str_data(&test_date));

    date->is_set_time_offset = false;
    str_put_c (&test_date, base_end, "-00:00");
    date_write_rfc3339 (date, buff);
    string_test (t, "Unknown UTC offset", buff, str_data(&test_date));

    parent_test_pop (t);
}

void date_write_test (struct test_ctx_t *t, struct date_t *date, enum reference_time_duration_t precision, char *expected)
{
    test_push (t, "%s", expected);

    string_t test_date = {0};
    str_set (&test_date, expected);
    size_t base_end = str_len(&test_date);

    char buff[date_max_len[precision]];

    date->is_set_time_offset = true;
    date->time_offset_hour = 6;
    date->time_offset_minute = 30;
    str_put_c (&test_date, base_end, "");
    date_write (date, precision, true, buff);
    string_test (t, "Force no UTC offset", buff, str_data(&test_date));

    if (precision < D_HOUR) {
        str_cat_c (&test_date, "T");
        base_end = str_len(&test_date);
    }

    date->is_set_time_offset = true;
    date->time_offset_hour = 0;
    date->time_offset_minute = 0;
    str_put_c (&test_date, base_end, "Z");
    date_write (date, precision, false, buff);
    string_test (t, "UTC offset", buff, str_data(&test_date));

    date->time_offset_hour = 6;
    date->time_offset_minute = 30;
    str_put_c (&test_date, base_end, "+06:30");
    date_write (date, precision, false, buff);
    string_test (t, "Positive UTC offset", buff, str_data(&test_date));

    date->time_offset_hour = -6;
    str_put_c (&test_date, base_end, "-06:30");
    date_write (date, precision, false, buff);
    string_test (t, "Negative UTC offset", buff, str_data(&test_date));

    date->is_set_time_offset = false;
    str_put_c (&test_date, base_end, "-00:00");
    date_write (date, precision, false, buff);
    string_test (t, "Unknown UTC offset", buff, str_data(&test_date));

    if (date->second_fraction > 0) {
        int_test (t, "Maximum length", strlen (buff), date_max_len[precision] - 1);
    }

    parent_test_pop (t);
}

void datetime_tests (struct test_ctx_t *t)
{
    test_push (t, "Date and Time");

    {
        test_push (t, "Date compare");
        struct date_t _d1 = {0};
        struct date_t *d1 = &_d1;
        struct date_t _d2 = {0};
        struct date_t *d2 = &_d2;

        date_set (d1, 1900, 8, 7, 2, 3, 4, 0.125, false, 0, 0);
        date_set (d2, 1900, 8, 7, 2, 3, 4, 0.125, false, 1, 1);
        date_compare_test (t, "Different but disabled UTC offset", d1, d2);

        date_set (d1, 1900, 8, 7, 2, 3, 4, 0.125, true, 0, 0);
        date_compare_test (t, "Exactly equal values", d1, d1);

        date_set (d2, 1901, 8, 7, 2, 3, 4, 0.125, true, 0, 0);
        different_date_compare_test (t, "Different year", d1, d2);
        date_set (d2, 1900, 1, 7, 2, 3, 4, 0.125, true, 0, 0);
        different_date_compare_test (t, "Different month", d1, d2);
        date_set (d2, 1900, 8, 1, 2, 3, 4, 0.125, true, 0, 0);
        different_date_compare_test (t, "Different day", d1, d2);
        date_set (d2, 1900, 8, 7, 1, 3, 4, 0.125, true, 0, 0);
        different_date_compare_test (t, "Different hour", d1, d2);
        date_set (d2, 1900, 8, 7, 2, 0, 4, 0.125, true, 0, 0);
        different_date_compare_test (t, "Different minute", d1, d2);
        date_set (d2, 1900, 8, 7, 2, 3, 0, 0.125, true, 0, 0);
        different_date_compare_test (t, "Different second", d1, d2);
        date_set (d2, 1900, 8, 7, 2, 3, 4, 0.000, true, 0, 0);
        different_date_compare_test (t, "Different second fraction", d1, d2);
        date_set (d2, 1900, 8, 7, 2, 3, 4, 0.125, false, 0, 0);
        different_date_compare_test (t, "Different has UTC offset", d1, d2);
        date_set (d2, 1900, 8, 7, 2, 3, 4, 0.125, true, 1, 0);
        different_date_compare_test (t, "Different time offset minute", d1, d2);
        date_set (d2, 1900, 8, 7, 2, 3, 4, 0.125, true, 0, 1);
        different_date_compare_test (t, "Different time offset minute", d1, d2);

        parent_test_pop (t);
    }

    {
        test_push (t, "Date read");
        struct date_t _expected = {0};
        struct date_t *expected = &_expected;

        // NOTE(sleon): Use seconds fraction of 0.125 because it's exactly
        // representable as floating point value, this means it can be safely
        // compared with equality and we don't depend on the rounding mode.

        // Normal timestamp format as defined by RFC3339
        date_set (expected, 1900, 8, 7, 2, 3, 4, 0.0, true, 0, 0);
        date_read_test (t, "1900-08-07 02:03:04Z", expected);
        date_set (expected, 1900, 8, 7, 2, 3, 4, 0.125, true, 0, 0);
        date_read_test (t, "1900-08-07 02:03:04.125Z", expected);

        // Permissive format where we allow all numeric components except year
        // to have fewer than 2 digits. Year must always be 4 digits.
        date_read_test (t, "1900-8-7 2:3:4.125Z", expected);

        // Special notation (not allowed by RFC3339). It allows varying
        // precision when omitting components at the end.
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

        // Invalid length for numeric elements
        invalid_date_read_test (t, "190-08-07 02:03:04.125Z");
        invalid_date_read_test (t, "19000-08-07 02:03:04.125Z");
        invalid_date_read_test (t, "1900-008-07 02:03:04.125Z");
        invalid_date_read_test (t, "1900-8-007 02:03:04.125Z");
        invalid_date_read_test (t, "1900-08-07 002:03:04.125Z");
        invalid_date_read_test (t, "1900-08-07 02:003:04.125Z");
        invalid_date_read_test (t, "1900-08-07 02:03:004.125Z");
        invalid_date_read_test (t, "1900-08-07 02:03:04.125+006:05");
        invalid_date_read_test (t, "1900-08-07 02:03:04.125+06:005");

        // Invalid missing components
        invalid_date_read_test (t, "1900-08-0702:03:04.125Z");
        invalid_date_read_test (t, "-08-07 02:03:04.125Z");
        invalid_date_read_test (t, "1900--07 02:03:04.125Z");
        invalid_date_read_test (t, "1900-08- 02:03:04.125Z");
        invalid_date_read_test (t, "1900-08-07 :03:04.125Z");
        invalid_date_read_test (t, "1900-08-07 02::04.125Z");
        invalid_date_read_test (t, "1900-08-07 02:03:.125Z");
        invalid_date_read_test (t, "1900-08-07 02:03:04.125+:05");
        invalid_date_read_test (t, "1900-08-07 02:03:04.125-:05");
        invalid_date_read_test (t, "1900-08-07 02:03:04.125-06:");
        invalid_date_read_test (t, "1900-08-07 02:03:04.Z");
        invalid_date_read_test (t, "08-07 02:03:04.125Z");
        invalid_date_read_test (t, "1900-07 02:03:04.125Z");
        invalid_date_read_test (t, "1900 02:03:04.125Z");
        invalid_date_read_test (t, "1900-08-07 03:04.125Z");
        invalid_date_read_test (t, "1900-08-07 04.125Z");
        invalid_date_read_test (t, "1900-08-07 02:03:04.125+05");
        invalid_date_read_test (t, "1900-08-07 02:03:04.125+");
        invalid_date_read_test (t, "1900-08-07 02:03:04.125-05");
        invalid_date_read_test (t, "1900-08-07 02:03:04.125-");

        parent_test_pop (t);
    }

    {
        test_push (t, "Date write as RFC339 timestamp");
        struct date_t _date = {0};
        struct date_t *date = &_date;

        date_set (date, 1900, 8, 7, 2, 3, 4, 0.0, false, 0, 0);
        date_write_rfc3339_test (t, date, "1900-08-07T02:03:04");

        // With second fraction
        date_set (date, 1900, 8, 7, 2, 3, 4, 1.0/3, false, 0, 0);
        date_write_rfc3339_test (t, date, "1900-08-07T02:03:04.333");

        parent_test_pop (t);
    }

    {
        test_push (t, "Date write");
        struct date_t _date = {0};
        struct date_t *date = &_date;

        date_set (date, 1900, 8, 7, 2, 3, 4, 1.0/3, false, 0, 0);
        date_write_test (t, date, D_SECOND, "1900-08-07T02:03:04.333");
        date_set (date, 1900, 8, 7, 2, 3, 4, 0.0, false, 0, 0);
        date_write_test (t, date, D_SECOND, "1900-08-07T02:03:04");
        date_write_test (t, date, D_MINUTE, "1900-08-07T02:03");
        date_write_test (t, date, D_HOUR,   "1900-08-07T02");
        date_write_test (t, date, D_DAY,    "1900-08-07");
        date_write_test (t, date, D_MONTH,  "1900-08");
        date_write_test (t, date, D_YEAR,   "1900");

        parent_test_pop (t);
    }

    {
        test_push (t, "Recurrent event tests");

        char res[DATE_TIMESTAMP_MAX_LEN];
        struct recurrent_event_t re = {0};

        {
            bool success = true;
            test_push (t, "Every 7th day");
            set_recurrent_event (&re, 7, D_DAY, NULL, "1908-1-4");
            //set_recurrent_event_compact (&re, "_-_-[7]", "1908-01-4");
            compute_next_occurence (&re, NULL, res);

            char *expected = "1908-01-11T-00:00";
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
