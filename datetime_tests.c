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
        str_cat_printf (t->error, "Failed date comparison:\n");
        str_date_internal (t->error, d1, d2);
        success = false;
    }
    test_pop (t, success);
}

void invalid_date_read_test (struct test_ctx_t *t, char *date_str)
{
    bool success = true;
    struct date_t res = {0};

    string_t message = {0};
    bool is_valid = date_read (date_str, &res, &message);

    test_push (t, "'%s' (%s)", date_str, str_data(&message));
    if (is_valid) {
        str_cat_printf (t->error, "Date should be invalid but isn't.\n");
        str_date_internal (t->error, &res, NULL);
        success = false;
    }
    str_free (&message);
    test_pop (t, success);
}

void date_to_utc_test (struct test_ctx_t *t, char *date_str, char *expected)
{
    bool success = true;
    struct date_t res = {0};
    string_t message = {0};

    test_push (t, "'%s' => '%s'", date_str, expected);
    struct date_t tmp = {0};
    bool is_valid = date_read (date_str, &tmp, &message);
    if (!is_valid) {
        str_cat_printf (t->error, "Valid source date '%s' marked as invalid:\n  '%s'\n", date_str, str_data(&message));
        success = false;
    }

    date_to_utc (&tmp, &res);

    struct date_t expected_d = {0};
    is_valid = date_read (expected, &expected_d, &message);
    if (!is_valid) {
        str_cat_printf (t->error, "Valid target UTC date '%s' marked as invalid:\n  '%s'\n", expected, str_data(&message));
        success = false;
    }

    if (date_cmp (&res, &expected_d) != 0) {
        str_cat_printf (t->error, "Failed date comparison:\n");
        str_date_internal (t->error, &res, &expected_d);
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

    } else if (expected != NULL && date_cmp (&res, expected) != 0) {
        str_cat_printf (t->error, "Wrong parsing of '%s':\n", date_str);
        str_date_internal (t->error, &res, expected);
        success = false;
    }
    str_free (&message);
    test_pop (t, success);
}

void valid_date_read_test (struct test_ctx_t *t, char *date_str)
{
    date_read_test_single (t, date_str, NULL);
}

void date_read_test_offset_types (struct test_ctx_t *t, string_t *test_date, struct date_t *expected)
{
    size_t base_end = str_len(test_date);

    // RFC3339 UTC offset
    str_put_c (test_date, base_end, "06:05");
    date_read_test_single (t, str_data(test_date), expected);

    // Non-padded
    str_put_c (test_date, base_end, "6:5");
    date_read_test_single (t, str_data(test_date), expected);

    // Non-colon ISO 8601
    str_put_c (test_date, base_end, "0605");
    date_read_test_single (t, str_data(test_date), expected);
}

void date_read_test_all_offsets (struct test_ctx_t *t, char *date_str, struct date_t *expected)
{
    struct date_t expected_l = *expected;

    string_t _test_date = {0};
    string_t *test_date = &_test_date;
    str_set (test_date, date_str);
    size_t base_end = str_len(test_date);

    // Zero UTC offset
    expected_l.is_set_utc_offset = true;
    expected_l.utc_offset_hour = 0;
    expected_l.utc_offset_minute = 0;
    str_put_c (test_date, base_end, "Z");
    date_read_test_single (t, str_data(test_date), &expected_l);
    str_put_c (test_date, base_end, "+00:00");
    date_read_test_single (t, str_data(test_date), &expected_l);

    // Positive UTC offset
    expected_l.utc_offset_hour = 6;
    expected_l.utc_offset_minute = 5;
    str_put_c (test_date, base_end, "+");
    date_read_test_offset_types (t, test_date, &expected_l);

    // Negative UTC offset
    expected_l.utc_offset_hour = -6;
    str_put_c (test_date, base_end, "-");
    date_read_test_offset_types (t, test_date, &expected_l);

    // Unknown UTC offset
    expected_l.is_set_utc_offset = false;
    str_put_c (test_date, base_end, "-00:00");
    date_read_test_single (t, str_data(test_date), &expected_l);
    str_put_c (test_date, base_end, "");
    date_read_test_single (t, str_data(test_date), &expected_l);

    str_free (test_date);
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

void date_write_rfc3339_test (struct test_ctx_t *t, struct date_t *date, char *expected)
{
    test_push (t, "%s", expected);

    string_t test_date = {0};
    str_set (&test_date, expected);
    size_t base_end = str_len(&test_date);

    char buff[date_max_len[D_SECOND]];

    date->is_set_utc_offset = true;
    date->utc_offset_hour = 0;
    date->utc_offset_minute = 0;
    str_put_c (&test_date, base_end, "Z");
    date_write_rfc3339 (date, buff);
    string_test (t, "UTC offset", buff, str_data(&test_date));

    date->utc_offset_hour = 6;
    date->utc_offset_minute = 30;
    str_put_c (&test_date, base_end, "+06:30");
    date_write_rfc3339 (date, buff);
    string_test (t, "Positive UTC offset", buff, str_data(&test_date));

    date->utc_offset_hour = -6;
    str_put_c (&test_date, base_end, "-06:30");
    date_write_rfc3339 (date, buff);
    string_test (t, "Negative UTC offset", buff, str_data(&test_date));

    date->is_set_utc_offset = false;
    str_put_c (&test_date, base_end, "-00:00");
    date_write_rfc3339 (date, buff);
    string_test (t, "Unknown UTC offset", buff, str_data(&test_date));

    str_free (&test_date);
    test_pop_parent (t);
}

void date_write_test (struct test_ctx_t *t, struct date_t *date, enum reference_time_duration_t precision, char *expected)
{
    test_push (t, "%s", expected);

    string_t test_date = {0};
    str_set (&test_date, expected);
    size_t base_end = str_len(&test_date);

    char buff[date_max_len[precision]];

    date->is_set_utc_offset = true;
    date->utc_offset_hour = 6;
    date->utc_offset_minute = 30;
    str_put_c (&test_date, base_end, "");
    date_write (date, precision, true, true, true, false, buff);
    string_test (t, "Force no UTC offset", buff, str_data(&test_date));

    if (precision < D_HOUR) {
        str_cat_c (&test_date, "T");
        base_end = str_len(&test_date);
    }

    date->is_set_utc_offset = true;
    date->utc_offset_hour = 0;
    date->utc_offset_minute = 0;
    str_put_c (&test_date, base_end, "Z");
    date_write (date, precision, false, true, true, false, buff);
    string_test (t, "UTC offset", buff, str_data(&test_date));

    date->utc_offset_hour = 6;
    date->utc_offset_minute = 30;
    str_put_c (&test_date, base_end, "+06:30");
    date_write (date, precision, false, true, true, false, buff);
    string_test (t, "Positive UTC offset", buff, str_data(&test_date));

    date->utc_offset_hour = -6;
    str_put_c (&test_date, base_end, "-06:30");
    date_write (date, precision, false, true, true, false, buff);
    string_test (t, "Negative UTC offset", buff, str_data(&test_date));

    date->is_set_utc_offset = false;
    str_put_c (&test_date, base_end, "-00:00");
    date_write (date, precision, false, true, true, false, buff);
    string_test (t, "Unknown UTC offset", buff, str_data(&test_date));

    if (precision != D_SECOND || date->second_fraction > 0) {
        int_test (t, "Maximum length", strlen(buff) + 1, date_max_len[precision]);
    }

    str_free (&test_date);
    test_pop_parent (t);
}

#define DATE_STR_L(date, date_str)       \
char date_str[DATE_TIMESTAMP_MAX_LEN];   \
{                                        \
    date_write_rfc3339 (date, date_str); \
}

void date_operation_test (struct test_ctx_t *t, struct date_t *d1, int value, enum reference_time_duration_t unit,
                          struct date_t *d2)
{
    DATE_STR_L (d1, d1_str);
    DATE_STR_L (d2, d2_str);
    test_push (t, "%s + %d(%s) = %s", d1_str, value, reference_time_duration_names[unit], d2_str);

    struct date_t _d = {0};
    struct date_t *d = &_d;

    *d = *d1;
    date_add_value (d, value, unit);
    date_compare_test (t, "d1 + v == d2", d, d2);

    *d = *d2;
    date_add_value (d, -value, unit);
    date_compare_test (t, "d2 - v == d1", d, d1);

    test_pop_parent (t);
}

bool date_get_day_of_week_test (struct test_ctx_t *t, char *test_name, struct date_t *date, enum day_of_week_t expected)
{
    bool success = true;

    int day = date_get_day_of_week (date);

    char buff[DATE_TIMESTAMP_MAX_LEN];
    date_write_compact (date, D_SECOND, buff);

    if (test_name == NULL) {
        test_push (t, "%s", buff);
    } else {
        test_push (t, "%s (%s)", test_name, buff);
    }

    if (day != expected) {
        success = false;
        str_cat_printf (t->error, "Wrong day of week got '%s', expected '%s'\n",
                        day_names[day], day_names[expected]);
    }
    test_pop (t, success);

    return success;
}

void date_generic_zellers_congruence_test (struct test_ctx_t *t, char *test_name,
                                           struct date_t *reference, struct date_t *date, int n,
                                           enum day_of_week_t expected)
{
    bool success = true;

    int day = date_generic_zellers_congruence (reference, date, n);

    char buff[DATE_TIMESTAMP_MAX_LEN];
    date_write_compact (date, D_SECOND, buff);

    if (test_name == NULL) {
        test_push (t, "%s", buff);
    } else {
        test_push (t, "%s (%s)", test_name, buff);
    }

    if (day != expected) {
        success = false;
        str_cat_printf (t->error, "Wrong day of week got '%s', expected '%s'\n",
                        day_names[day], day_names[expected]);
    }
    test_pop (t, success);
}

void date_recurrent_event_test (struct test_ctx_t *t, char *test_name,
                                int frequency, enum reference_time_duration_t scale,
                                struct date_element_t *date_element, struct date_t *start_date,
                                struct date_t *expected)
{
    string_t error = {0};
    bool success = true;
    struct date_t result = {0};
    struct recurrent_event_t re = {0};

    test_push (t, "%s", test_name);

    success = recurrent_event_set (&re,
                                   frequency, scale, date_element,
                                   start_date,
                                   &error);

    test_push (t, "Validate recurrent event definition");
    if (!success) {
        str_cat_printf (t->error, "Invalid recurrent event definition:\n  %s\n", str_data (&error));
    }
    test_pop (t, success);

    if (success) {
        recurrent_event_next (&re, NULL, &result);
        date_compare_test (t, "Compute next", &result, expected);
    }

    test_pop_parent (t);
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

        test_pop_parent (t);
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
        date_read_test (t, "1900-08-07 02:03:04", expected);
        date_set (expected, 1900, 8, 7, 2, 3, 4, 0.125, true, 0, 0);
        date_read_test (t, "1900-08-07 02:03:04.125", expected);

        // Permissive format where we allow all numeric components except year
        // to have fewer than 2 digits. Year must always be 4 digits.
        date_read_test (t, "1900-8-7 2:3:4.125", expected);

        // Special notation (not allowed by RFC3339). It allows varying
        // precision when omitting components at the end.
        date_set (expected, 1900, 1, 1, 20, 30, -1, 0.0, true, 0, 0);
        date_read_test (t, "1900-01-01 20:30", expected);
        date_set (expected, 1900, 1, 1, 20, -1, -1, 0.0, true, 0, 0);
        date_read_test (t, "1900-01-01 20", expected);
        date_set (expected, 1900, 1, 1, -1, -1, -1, 0.0, true, 0, 0);
        date_read_test (t, "1900-01-01 ", expected);
        date_set (expected, 1900, 1, -1, -1, -1, -1, 0.0, true, 0, 0);
        date_read_test (t, "1900-01 ", expected);
        date_set (expected, 1900, -1, -1, -1, -1, -1, 0.0, true, 0, 0);
        date_read_test (t, "1900 ", expected);

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
        invalid_date_read_test (t, "1900-08-07 02:03:04.125-06:-05");
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

        test_pop_parent (t);
    }

    {
        test_push (t, "Date operations");

        date_operation_test (t,
            DATE_P(2015, 6, 30, -1, -1, -1, 0.0, false, 0, 0),
            1, D_DAY,
            DATE_P(2015, 7,  1, -1, -1, -1, 0.0, false, 0, 0));

        date_operation_test (t,
            DATE_P(1900, 12, 31, 19, 59, 59, 0.0, false, 0, 0),
            5, D_HOUR,
            DATE_P(1901,  1,  1,  0, 59, 59, 0.0, false, 0, 0));

        date_operation_test (t,
            DATE_P(1900, 12, 31, 23, 59, 59, 0.25, false, 0, 0),
            1, D_SECOND,
            DATE_P(1901,  1,  1,  0,  0,  0, 0.25, false, 0, 0));

        // Make sure leap days are taken into account
        date_operation_test (t,
            DATE_P(1900, 2, 28, 19, 59, 59, 0.0, false, 0, 0),
            1, D_DAY,
            DATE_P(1900, 3,  1, 19, 59, 59, 0.0, false, 0, 0));

        date_operation_test (t,
            DATE_P(2000, 2, 28, 19, 59, 59, 0.0, false, 0, 0),
            1, D_DAY,
            DATE_P(2000, 2, 29, 19, 59, 59, 0.0, false, 0, 0));

        date_operation_test (t,
            DATE_P(2000, 2, 29, 19, 59, 59, 0.0, false, 0, 0),
            1, D_DAY,
            DATE_P(2000, 3,  1, 19, 59, 59, 0.0, false, 0, 0));

        // Make sure leap seconds are taken into account
        date_operation_test (t,
            DATE_P(2005, 12, 31, 23, 59, 59, 0.0, false, 0, 0),
            1, D_SECOND,
            DATE_P(2005, 12, 31, 23, 59, 60, 0.0, false, 0, 0));

        date_operation_test (t,
            DATE_P(2005, 12, 31, 23, 59, 60, 0.0, false, 0, 0),
            1, D_SECOND,
            DATE_P(2006,  1,  1,  0,  0,  0, 0.0, false, 0, 0));

        date_operation_test (t,
            DATE_P(2006, 12, 31, 23, 59, 59, 0.0, false, 0, 0),
            1, D_SECOND,
            DATE_P(2007,  1,  1,  0,  0,  0, 0.0, false, 0, 0));

        // :unrestricted_date_addition
        //date_operation_test (t,
        //    DATE_P(1900, 1, 1, 0, 0, 0, 0.0, false, 0, 0),
        //    120, D_MINUTE,
        //    DATE_P(1900, 1, 1, 2, 0, 0, 0.0, false, 0, 0));

        test_pop_parent (t);
    }

    {
        test_push (t, "Date to UTC");

        date_to_utc_test (t, "1900-12-31 18:00:00-05:00",
                             "1900-12-31 23:00:00Z");

        date_to_utc_test (t, "1900-12-31 19:59:59-05:00",
                             "1901-01-01 00:59:59Z");

        date_to_utc_test (t, "1900-01-01 05:00:00+05:00",
                             "1900-01-01 00:00:00Z");

        date_to_utc_test (t, "1900-01-01 04:59:59+05:00",
                             "1899-12-31 23:59:59Z");

        date_to_utc_test (t, "1900-12-31 19:30:00-03:30",
                             "1900-12-31 23:00:00Z");

        date_to_utc_test (t, "1900-12-31 21:29:59-03:30",
                             "1901-01-01 00:59:59Z");

        date_to_utc_test (t, "1900-01-01 03:30:00+03:30",
                             "1900-01-01 00:00:00Z");

        date_to_utc_test (t, "1900-01-01 03:29:59+03:30",
                             "1899-12-31 23:59:59Z");

        date_to_utc_test (t, "1900-12-31 20:30:00-03:30",
                             "1901-01-01 00:00:00Z");

        date_to_utc_test (t, "1996-12-19T16:39:57-08:00",
                             "1996-12-20T00:39:57Z");

        date_to_utc_test (t, "1990-12-31T15:59:60-08:00",
                             "1990-12-31T23:59:60Z");

        test_pop_parent (t);
    }

    {
        test_push (t, "Date value validation");

        // Leap year tests
        invalid_date_read_test (t, "1900-02-29");
        valid_date_read_test   (t, "2000-02-29");
        invalid_date_read_test (t, "2009-02-29");
        invalid_date_read_test (t, "2010-02-29");
        invalid_date_read_test (t, "2011-02-29");
        valid_date_read_test   (t, "2012-02-29");
        invalid_date_read_test (t, "2100-02-29");

        invalid_date_read_test (t, "2010-13-31");
        invalid_date_read_test (t, "2010-00-31");
        invalid_date_read_test (t, "2010-02-32");
        invalid_date_read_test (t, "2010-02-00");
        invalid_date_read_test (t, "2010-02-31");
        invalid_date_read_test (t, "2010-02-30");
        invalid_date_read_test (t, "2010-04-31");
        invalid_date_read_test (t, "2010-06-31");
        invalid_date_read_test (t, "2010-09-31");
        invalid_date_read_test (t, "2010-11-31");

        invalid_date_read_test (t, "1900-01-01 24:00:00");
        invalid_date_read_test (t, "1900-01-01 23:60:00");
        invalid_date_read_test (t, "1900-01-01 23:59:60");
        invalid_date_read_test (t, "1900-01-01 23:59:59+24:00");
        invalid_date_read_test (t, "1900-01-01 23:59:59+00:60");
        invalid_date_read_test (t, "1900-01-01 23:59:59-24:00");
        invalid_date_read_test (t, "1900-01-01 23:59:59-00:60");

        // Leap second tests
        valid_date_read_test   (t, "2005-12-31 23:59:60");
        valid_date_read_test   (t, "2005-12-31 23:59:60Z");
        invalid_date_read_test (t, "2005-12-31 23:59:60-06:00");
        valid_date_read_test   (t, "2005-12-31 20:29:60-03:30");
        valid_date_read_test   (t, "2006-01-01 03:29:60+03:30");
        invalid_date_read_test (t, "2006-01-01 03:29:60Z");
        valid_date_read_test   (t, "2015-06-30 23:59:60");
        valid_date_read_test   (t, "2015-06-30 23:59:60Z");
        invalid_date_read_test (t, "2015-06-30 23:59:60-06:00");
        valid_date_read_test   (t, "2015-06-30 20:29:60-03:30");
        valid_date_read_test   (t, "2015-07-01 03:29:60+03:30");
        invalid_date_read_test (t, "2015-07-01 03:29:60Z");

        test_pop_parent (t);
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

        // Use the smallest value for unknown components.
        date_set (date, 1900, 8, 7, 2, 3, -1, 0.0, false, 0, 0);
        date_write_rfc3339_test (t, date, "1900-08-07T02:03:00");

        date->minute = -1;
        date_write_rfc3339_test (t, date, "1900-08-07T02:00:00");

        date->hour = -1;
        date_write_rfc3339_test (t, date, "1900-08-07T00:00:00");

        date->day = -1;
        date_write_rfc3339_test (t, date, "1900-08-01T00:00:00");

        date->month = -1;
        date_write_rfc3339_test (t, date, "1900-01-01T00:00:00");

        test_pop_parent (t);
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

        test_pop_parent (t);
    }

    {
        test_push (t, "Date write (available and requested precision mismatch)");
        struct date_t _date = {0};
        struct date_t *date = &_date;

        char buff[DATE_TIMESTAMP_MAX_LEN];

        date_set (date, 1900, 8, 7, 2, 3, -1, -1, true,-6, 30);
        date_write_compact (date, D_SECOND, buff);
        string_test (t, "Minute Precision", buff, "1900-8-7T2:3-06:30");

        date_set (date, 1900, 8, 7, 2, -1, -1, -1, true,-6, 30);
        date_write_compact (date, D_SECOND, buff);
        string_test (t, "Hour Precision", buff, "1900-8-7T2-06:30");

        date_set (date, 1900, 8, 7, -1, -1, -1, -1, true,-6, 30);
        date_write_compact (date, D_SECOND, buff);
        string_test (t, "Day Precision", buff, "1900-8-7T-06:30");

        date_set (date, 1900, 8, -1, -1, -1, -1, -1, true,-6, 30);
        date_write_compact (date, D_SECOND, buff);
        string_test (t, "Month Precision", buff, "1900-8T-06:30");

        date_set (date, 1900, -1, -1, -1, -1, -1, -1, true,-6, 30);
        date_write_compact (date, D_SECOND, buff);
        string_test (t, "Year Precision", buff, "1900T-06:30");

        test_pop_parent (t);
    }

    {
        test_push (t, "Get day of week");

        date_get_day_of_week_test (t, NULL,
                                   &DATE_DAY(2000, 1, 1),
                                   D_SATURDAY);

        date_get_day_of_week_test (t, NULL,
                                   &DATE_DAY(2000, 3, 1),
                                   D_WEDNESDAY);

        struct date_t now;
        date_get_now_d (&now);

        struct tm local_time = {0};
        time_t now_seconds = time(NULL);
        localtime_r (&now_seconds, &local_time);

        date_get_day_of_week_test (t, "Today",
                                   &now,
                                   local_time.tm_wday);

        test_pop_parent (t);
    }

    {
        test_push (t, "Generic Zeller's congruence");

        struct date_t first_sunday = DATE_DAY(0, 3, 5);
        enum day_of_week_t expected = date_get_day_of_week (&first_sunday);
        date_generic_zellers_congruence_test (t, NULL,
            &first_sunday,
            &first_sunday,
            7,
            expected);
        assert (expected == D_SUNDAY);

        struct date_t date = DATE_DAY(0, 3, 1);
        expected = date_get_day_of_week (&date);
        date_generic_zellers_congruence_test (t, NULL,
            &first_sunday,
            &date,
            7,
            expected);

        date = DATE_DAY(1600, 3, 12);
        expected = date_get_day_of_week (&date);
        date_generic_zellers_congruence_test (t, NULL,
            &first_sunday,
            &date,
            7,
            expected);

        date = DATE_DAY(2000, 1, 1);
        expected = date_get_day_of_week (&date);
        date_generic_zellers_congruence_test (t, NULL,
            &first_sunday,
            &date,
            7,
            expected);

        date = DATE_DAY(2000, 3, 1);
        expected = date_get_day_of_week (&date);
        date_generic_zellers_congruence_test (t, NULL,
            &first_sunday,
            &date,
            7,
            expected);

        test_pop_parent (t);
    }

    {
        test_push (t, "Generic Zeller's and day addition ");

        bool success = true;
        struct date_t start = DATE_DAY(1600, 1, 3);
        struct date_t end = DATE_DAY(1601, 1, 22);
        int step = 11;

        enum day_of_week_t start_day_of_week = date_get_day_of_week (&start);

        int day_count = 0;
        struct date_t curr_date = start;
        while (success && date_cmp (&curr_date, &end) <= 0) {
            int equivalence_class = date_generic_zellers_congruence (&start, &curr_date, step);

            char buff[DATE_TIMESTAMP_MAX_LEN];
            date_write (&curr_date, D_SECOND, false, true, true, true, buff);
            test_push (t, "Test equivalence class (%s -> %d)", buff, date_get_absolute_day_number (&curr_date));

            if (equivalence_class != 0) {
                success = false;
                str_cat_printf (t->error, "Day addition and generic Zeller's equivalence class don't match, got %d.\n", equivalence_class);
                date_write (&start, D_SECOND, false, true, true, true, buff);
                str_cat_printf (t->error, " Start: '%s' -> %d\n", buff, date_get_absolute_day_number (&start));
                date_write (&curr_date, D_SECOND, false, true, true, true, buff);
                str_cat_printf (t->error, "  Curr: '%s' -> %d\n", buff, date_get_absolute_day_number (&curr_date));
            }

            test_pop (t, success);

            if (success) {
                if (day_count % (step*7) == 0) {
                    success = date_get_day_of_week_test (t, "Test we reached the starting weekday", &curr_date, start_day_of_week);
                }

                date_add_value_restricted (&curr_date, step, D_DAY);
                day_count += step;
            }
        }

        test_pop_parent (t);
    }

    {
        test_push (t, "Recurrent event tests");

        date_recurrent_event_test (t, "Saturdays",
            7, D_DAY, NULL,
            &DATE_DAY(1908, 1, 4),

            &DATE_DAY (1908, 1, 11));

        date_recurrent_event_test (t, "International worker's day",
            1, D_YEAR, &DATE_ELEMENT_DAY(-1, 5, 1),
            &DATE_YEAR(1890),

            &DATE_DAY (1891, 5, 1));

        date_recurrent_event_test (t, "Every month",
            1, D_MONTH, &DATE_ELEMENT_DAY(-1, -1, 15),
            &DATE_MONTH(2000, 2),

            &DATE_DAY (2000, 3, 15));

        date_recurrent_event_test (t, "Every 2 months",
            2, D_MONTH, &DATE_ELEMENT_DAY(-1, -1, 20),
            &DATE_MONTH(2000, 2),

            &DATE_DAY (2000, 4, 20));

        date_recurrent_event_test (t, "Every 2 months starting January",
            2, D_MONTH, &DATE_ELEMENT_DAY(-1, -1, 20),
            &DATE_MONTH(2000, 1),

            &DATE_DAY (2000, 3, 20));

        test_pop_parent (t);
    }

    test_pop_parent (t);
}
