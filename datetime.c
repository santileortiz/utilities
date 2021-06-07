/*
 * Copyright (C) 2019 Santiago León O.
 */

// These are utility functions to properly handle date and time information.
//
// As a format for date and time we will use sequences of ASCII characters
// somewhat based on what XML does, based on ISO 8601.
//
// In general we will have to always be careful about compatibility of date
// formats if we ever decide to export to real XML or JSON.
// 
// At the moment no compatibility guarantees are made about these functions
// with respect to other libraries or standards. First we want to guearantee
// we are consistent with ourselves.

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif

#include <time.h>

enum reference_time_duration_t {
    D_YEAR,
    D_MONTH,
    D_DAY,
    D_HOUR,
    D_MINUTE,
    D_SECOND
};

// Is endianess going to affect the order of these? They need to match so that
// date->year == date->v[D_YEAR]
struct date_t {
    union {
        struct {
            int32_t year;
            int32_t month;
            int32_t day;
            int32_t hour;
            int32_t minute;
            int32_t second;
        };
        int32_t v[6];
    };

    double second_fraction;

    bool is_set_time_offset;
    int32_t time_offset_hour;
    int32_t time_offset_minute;
};

struct date_scanner_t {
    char *pos;
    bool is_eof;

    bool error;
    char *error_message;
};

void date_set (struct date_t *d,
               int year, int month, int day,
               int hour, int minute, int second, double second_fraction,
               bool is_set_time_offset, int time_offset_hour, int time_offset_minute)
{
    d->year = year;
    d->month = month;
    d->day = day;
    d->hour = hour;
    d->minute = minute;
    d->second = second;
    d->second_fraction = second_fraction;
    d->is_set_time_offset = is_set_time_offset;
    d->time_offset_hour = time_offset_hour;
    d->time_offset_minute = time_offset_minute;
}
#define BOOL_STR(value) (value) ? "true" : "false"

void str_date_internal (string_t *str, struct date_t *d, struct date_t *expected)
{
    str_cat_printf (str, " year: %d", d->year);
    (expected != NULL && d->year != expected->year) ?  str_cat_printf (str, " (expected %d)\n", expected->year) : str_cat_printf (str, "\n");
    str_cat_printf (str, " month: %d", d->month);
    (expected != NULL && d->month != expected->month) ?  str_cat_printf (str, " (expected %d)\n", expected->month) : str_cat_printf (str, "\n");
    str_cat_printf (str, " day: %d", d->day);
    (expected != NULL && d->day != expected->day) ?  str_cat_printf (str, " (expected %d)\n", expected->day) : str_cat_printf (str, "\n");
    str_cat_printf (str, " hour: %d", d->hour);
    (expected != NULL && d->hour != expected->hour) ?  str_cat_printf (str, " (expected %d)\n", expected->hour) : str_cat_printf (str, "\n");
    str_cat_printf (str, " minute: %d", d->minute);
    (expected != NULL && d->minute != expected->minute) ?  str_cat_printf (str, " (expected %d)\n", expected->minute) : str_cat_printf (str, "\n");
    str_cat_printf (str, " second: %d", d->second);
    (expected != NULL && d->second != expected->second) ?  str_cat_printf (str, " (expected %d)\n", expected->second) : str_cat_printf (str, "\n");
    str_cat_printf (str, " second_fraction: %f", d->second_fraction);
    (expected != NULL && d->second_fraction != expected->second_fraction) ?  str_cat_printf (str, " (expected %f)\n", expected->second_fraction) : str_cat_printf (str, "\n");
    str_cat_printf (str, " is_set_time_offset: %s", BOOL_STR(d->is_set_time_offset));
    (expected != NULL && d->is_set_time_offset != expected->is_set_time_offset) ?  str_cat_printf (str, " (expected %s)\n", BOOL_STR(expected->is_set_time_offset)) : str_cat_printf (str, "\n");
    str_cat_printf (str, " time_offset_hour: %d", d->time_offset_hour);
    (expected != NULL && d->time_offset_hour != expected->time_offset_hour) ?  str_cat_printf (str, " (expected %d)\n", expected->time_offset_hour) : str_cat_printf (str, "\n");
    str_cat_printf (str, " time_offset_minute: %d", d->time_offset_minute);
    (expected != NULL && d->time_offset_minute != expected->time_offset_minute) ?  str_cat_printf (str, " (expected %d)\n", expected->time_offset_minute) : str_cat_printf (str, "\n");
}

bool date_scanner_int (struct date_scanner_t *scnr, int32_t *value)
{
    assert (value != NULL);
    if (scnr->error) return false;

    if (!isdigit (*scnr->pos)) return false;

    char *end;
    int res = strtol (scnr->pos, &end, 10);
    if (scnr->pos != end) {
        *value = res;
        scnr->pos = end;

        if (*scnr->pos == '\0') {
            scnr->is_eof = true;
        }
        return true;
    }

    return false;
}

bool date_scanner_double (struct date_scanner_t *scnr, double *value)
{
    assert (value != NULL);
    if (scnr->error) return false;

    char *end;
    double res = strtod (scnr->pos, &end);
    if (scnr->pos != end) {
        *value = res;
        scnr->pos = end;

        if (*scnr->pos == '\0') {
            scnr->is_eof = true;
        }
        return true;
    }

    return false;
}

bool date_scanner_char (struct date_scanner_t *scnr, char c)
{
    if (scnr->error) return false;

    if (*scnr->pos == c) {
        scnr->pos++;

        if (*scnr->pos == '\0') {
            scnr->is_eof = true;
        }

        return true;
    }

    return false;
}

void date_scan_numeric_offset (struct date_scanner_t *scnr,
                               bool *is_set_time_offset, int *time_offset_hour, int *time_offset_minute)
{
    if (date_scanner_char (scnr, '+') || date_scanner_char (scnr, '-')) {
        *is_set_time_offset = true;

        bool is_negative = *(scnr->pos - 1) == '-';

        if (!scnr->is_eof) {
            date_scanner_int (scnr, time_offset_hour);
            if (is_negative) *time_offset_hour = -(*time_offset_hour);
        }

        if (!scnr->is_eof) {
            if (date_scanner_char (scnr, ':')) {
                date_scanner_int (scnr, time_offset_minute);
            }
        }

        // Following section 4.3. of RFC3339, interpret -00:00 as unknown
        // offset to UTC.
        if (is_negative && *time_offset_hour == 0 && *time_offset_minute == 0)
        {
            *is_set_time_offset = false;
        }
    }
}

void backtrack_and_scan_offset (struct date_scanner_t *scnr, char *date_time_str,
                                bool *is_set_time_offset, int *time_offset_hour, int *time_offset_minute)
{
    while (scnr->pos >= date_time_str && *(scnr->pos) != '-') scnr->pos--;

    date_scan_numeric_offset (scnr, is_set_time_offset, time_offset_hour, time_offset_minute);
    if (!scnr->is_eof)
    {
        // Something after offset, error...
    }
}

bool date_read (char *date_time_str, struct date_t *date)
{
    assert (date_time_str != NULL && date != NULL);

    struct date_scanner_t scnr = {0};
    scnr.pos = date_time_str;

    bool is_set_time_offset = false;
    int time_offset_hour = 0;
    int time_offset_minute = 0;

    int32_t year;
    date_scanner_int (&scnr, &year);
    if (scnr.pos - date_time_str != 4) {
        // Non 4 digit year...
    }

    int32_t month = -1;
    if (!scnr.is_eof) {
        if (date_scanner_char (&scnr, '-')) {
            date_scanner_int (&scnr, &month);
        }
    }

    int32_t day = -1;
    if (!scnr.is_eof) {
        if (date_scanner_char (&scnr, '-')) {
            date_scanner_int (&scnr, &day);

        } else if (date_scanner_char (&scnr, ':')) {
            month = -1;
            backtrack_and_scan_offset (&scnr, date_time_str, &is_set_time_offset, &time_offset_hour, &time_offset_minute);
        }
    }

    int32_t hour = -1;
    if (!scnr.is_eof) {
        if (date_scanner_char (&scnr, ' ') || date_scanner_char (&scnr, 'T') || date_scanner_char (&scnr, 't')) {
            date_scanner_int (&scnr, &hour);

        } else if (date_scanner_char (&scnr, ':')) {
            day = -1;
            backtrack_and_scan_offset (&scnr, date_time_str, &is_set_time_offset, &time_offset_hour, &time_offset_minute);
        }
    }

    int32_t minute = -1;
    if (!scnr.is_eof) {
        if (date_scanner_char (&scnr, ':')) {
            date_scanner_int (&scnr, &minute);
        }
    }

    int32_t second = -1;
    if (!scnr.is_eof) {
        if (date_scanner_char (&scnr, ':')) {
            date_scanner_int (&scnr, &second);
        }
    }

    double second_fraction = 0.0;
    if (!scnr.is_eof) {
        if (date_scanner_char (&scnr, '.')) {
            scnr.pos--;
            date_scanner_double (&scnr, &second_fraction);
        }
    }

    if (!scnr.is_eof) {
        if (date_scanner_char (&scnr, 'Z') || date_scanner_char (&scnr, 'z')) {
            is_set_time_offset = true;

        } else {
            date_scan_numeric_offset (&scnr, &is_set_time_offset, &time_offset_hour, &time_offset_minute);
        }
    }

    if (!scnr.error) {
        date_set (date, year, month, day, hour, minute, second, second_fraction, is_set_time_offset, time_offset_hour, time_offset_minute);
    }

    return !scnr.error;
}

int date_cmp (struct date_t *d1, struct date_t *d2)
{
    if (d1->is_set_time_offset && d2->is_set_time_offset &&
        (d1->time_offset_hour != d2->time_offset_hour || d1->time_offset_minute != d2->time_offset_minute) ) {
        // TODO: normalize dates to UTC then compare.
        return -1;
    }

    int diff = d1->year - d2->year;

    if (diff == 0) {
        diff = d1->month - d2->month;
    }

    if (diff == 0) {
        diff = d1->day - d2->day;
    }

    if (diff == 0) {
        diff = d1->hour - d2->hour;
    }

    if (diff == 0) {
        diff = d1->minute - d2->minute;
    }

    if (diff == 0) {
        diff = d1->second - d2->second;
    }

    if (diff == 0) {
        diff = d1->second_fraction - d2->second_fraction;
    }

    if (diff == 0 && d1->is_set_time_offset)  {
        if (d1->is_set_time_offset == d2->is_set_time_offset) {
            if (diff == 0) {
                diff = d1->time_offset_hour - d2->time_offset_hour;
            }

            if (diff == 0) {
                diff = d1->time_offset_minute - d2->time_offset_minute;
            }

        } else {
            diff = -1;
        }
    }

    return diff;
}

// Define our own type in case we want to provide our own in memory
// representaton different than struct tm. We won't allow uses of this struct
// outside of this file. All code must use string dates. If it becomes a
// performance bottleneck we may reconsider this.
typedef struct tm timedate_t;

#define DATE_TIME_FORMAT "%Y-%m-%d %H:%M:%S%z"
#define DATE_TIME_LEN 24

#define DATE_FORMAT "%Y-%m-%d"
#define DATE_LEN 10

char *month_names[] = {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};

#define get_current_date_time_arr(arr) get_current_date_time (arr,ARRAY_SIZE(arr))
void get_current_date_time (char *str, size_t buff_size)
{
    assert (buff_size > DATE_TIME_LEN);

    time_t t = time(NULL);
    struct tm local_time = {0};
    if (!localtime_r (&t, &local_time)) {
        // We will assume this never happens.
        invalid_code_path;
    }

    strftime (str, buff_size, DATE_TIME_FORMAT, &local_time);
}

void read_date (char *date_time_str, timedate_t *timedate)
{
    if (!strptime (date_time_str, DATE_TIME_FORMAT, timedate)) {
        if (!strptime (date_time_str, DATE_FORMAT, timedate)) {
            // This is a fatal parsing error.
            invalid_code_path;
        }
    }
}

bool date_is_valid (int year, int month, int day, string_t *error)
{
    bool is_valid = true;

    if (year < 1582) {
        str_cat_printf (error, "%d is not a valid year.", year);
        if (year > 0) {
            str_cat_c (error, " Only the Gregorian calendar is supported. Dates before the Gregorian reform established on 15 October 1582 are most likely wrong.");
        }
        is_valid = false;
    }

    if (is_valid) {
        if (month < 1 || month > 12) {
            str_cat_printf (error, "%d is not a valid month.", month);
            is_valid = false;
        }
    }

    if (is_valid) {
        if (day < 1 || day > 31) {
            str_cat_printf (error, "%d is not a valid day.", day);
            is_valid = false;
        }
    }

    if (is_valid) {
        if ((month == 4 ||
            month == 6 ||
            month == 9 ||
            month == 11) &&
            (day > 30)) {
            str_cat_printf (error, "Day %d is not valid for %s.", day, month_names[month-1]);
            is_valid = false;
        }
    }

    if (is_valid) {
        if (month == 2) {
            bool is_leap_year = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
            if (!is_leap_year && day > 28) {
                str_cat_printf (error, "Day %d is not valid for %s.", day, month_names[month-1]);
                if (day == 29) {
                    str_cat_printf (error, " %d isn't a leap year.", year);
                }
                is_valid = false;

            } else if (is_leap_year && day > 29) {
                str_cat_printf (error, "Day %d is not valid for %s.", day, month_names[month-1]);
                is_valid = false;
            }
        }
    }

    return is_valid;
}

void read_date_plain (char *date_time_str, int *year, int *month, int *day)
{
    timedate_t timedate = {0};
    read_date (date_time_str, &timedate);
    *day = timedate.tm_mday;
    *month = timedate.tm_mon + 1;
    *year = timedate.tm_year + 1900;
}

// TODO: A date_time_reformat() function would be better and more 'generic' but
// there doesn't seem to be a straightforward way to know what the end size of a
// specific format would be, so it can be correctly allocated beforehand. There
// is also no way of distinguishing the case where the buffer was too small and
// where the format yielded an empty string.
char* date_time_to_date (mem_pool_t *pool, char *date_time_str)
{
    char *date = pom_push_size (pool, DATE_LEN+1);
    timedate_t timedate = {0};

    // FIXME: We assume that a shorter datetime string will be a date string. In
    // reality we would like to support varying levels of precission with
    // datetime strings. Year only, year and month, date, date and hour etc.
    // :shorter_date_time_is_date
    if (strlen(date_time_str) == DATE_TIME_LEN) {
        read_date (date_time_str, &timedate);

        strftime (date, DATE_LEN+1, DATE_FORMAT, &timedate);
    } else {
        strncpy (date, date_time_str, DATE_LEN+1);
    }
    return date;
}

void date_time_to_date_str (char *date_time_str, string_t *str)
{
    str_maybe_grow (str, DATE_LEN+1, false);

    char *dest = str_data(str);
    timedate_t timedate = {0};

    // :shorter_date_time_is_date
    if (strlen(date_time_str) == DATE_TIME_LEN) {
        read_date (date_time_str, &timedate);

        strftime (dest, DATE_LEN+1, DATE_FORMAT, &timedate);
    } else {
        strncpy (dest, date_time_str, DATE_LEN+1);
    }
}

int date_compare (char *d1, char *d2)
{
    timedate_t date1, date2;

    read_date (d1, &date1);
    read_date (d2, &date2);

    int res = 0;
    if (date1.tm_year < date2.tm_year) {
        res = -1;
    } else if (date1.tm_year > date2.tm_year) {
        res = 1;
    } else if (date1.tm_mon < date2.tm_mon) {
        res = -1;
    } else if (date1.tm_mon > date2.tm_mon) {
        res = 1;
    } else if (date1.tm_mday < date2.tm_mday) {
        res = -1;
    } else if (date1.tm_mday > date2.tm_mday) {
        res = 1;
    }

    return res;
}

struct recurrent_event_t {
    // I think frequence can be stored inside date_element, will always have
    // spare int there that are not used due to the choice of scale.
    int frequence;
    enum reference_time_duration_t scale;
    timedate_t date_element;
    timedate_t start;

    int count;
    timedate_t end;
};

void set_recurrent_event (struct recurrent_event_t *re, int frequence, enum reference_time_duration_t scale, char *date_element, char *start_date)
{
    assert (re != NULL);

    if (frequence < 2) frequence = 1;
    re->frequence = frequence;

    re->scale = scale;

    read_date (start_date, &re->start);
}

bool compute_next_occurence (struct recurrent_event_t *re, char *curr_occurence, char *next_occurence)
{
    assert (re != NULL && next_occurence != NULL);

    timedate_t current_td;
    if (curr_occurence == NULL) {
        current_td = re->start;
    } else {
        read_date (curr_occurence, &current_td);
    }
    
    if (re->scale == D_DAY) {
        current_td.tm_mday += re->frequence;
    }

    strftime (next_occurence, DATE_LEN+1, DATE_FORMAT, &current_td);

    return false;
}
