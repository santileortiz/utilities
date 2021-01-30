/*
 * Copyright (C) 2019 Santiago León O.
 */
#include "boilerplate.c"

////////////////////////////////////////////////////////////////////////////////
// API V2
// 288 LOC
//
// This is my second attempt at a scanner API. This was the one I used in the
// tokenizer for the full xkb file parser, and in the geometry string
// description parser. I also ported previous usage code extract_keymap_info()
// into this API.
//
// Pros:
//  - Code that uses it is much more readable
//  - Has support for error reporting
//  - Has line counting
//
// Cons:
//  - Line count was added late in the design so it will break in some specific
//    cases. We should really use a function to move the position of the scanner
//    forwards (and another for backwards? not sure if useful), this way there
//    is a central place where we can count the number of line breaks.
//
//  - At the beginning looked like adding a user configurable flag to make EOF
//    an error would be useful. In practice I almost never cared about this and
//    almost always enabled. As we use this in more places I think I will have a
//    better idea about where to assert and where to just return false as match
//    status. This way we can handle this semantics automatically and not bother
//    the user to think about it. Maybe we should never set the error value, let
//    that task always to the user, still the user may want a way to distinguish
//    an EOF from a failed match.
//
//  - There is still some work left to do on how to structure conditional stuff.

struct scanner_t {
    char *pos;
    bool is_eof;

    // TODO: Currently this is only set by the caller, I would like to handle
    // this internally in the scanner. Whenever the caller expects to match on a
    // specific thing and EOF is found. For example when a call to scanner_str
    // or scanner_char is made and instead of the expected characters we get an
    // EOF. But, this will not happen for scanner_int, here an EOF before we get
    // a digit would be an error, but in the middle it would just mark the end
    // of the number. This is related to the optional scanning decisions I have
    // yet to make. I kind of liked the approach I used here in the parser where
    // I have a consume function that can be split into an advance and an expect
    // function. Spend more time on this.
    bool eof_is_error;

    bool error;
    char *error_message;

    // TODO: Wrap the advancement of pos into a function so that we can be more
    // certain we correctly keep track of line numbers. This would also allow us
    // to keep track of the column number.
    int line_number;
};

// Calling scanning functions will never set the error flag, it's the
// responsability of the caller to call scanner_set_error() if a match should
// have happened but didn't. To allow this the return value of scanning
// functions is a boolean that is true if the match was successful and false
// otherwise.
//
// After an error is set, the return value for scanning functions  will always
// be false. This allows easy termination of the execution without adding gotos
// everywhere in the code.
//
// NOTE: The error message is not duplicated or stored by the scanner, it just
// stores a pointer to it.
void scanner_set_error (struct scanner_t *scnr, char *error_message)
{
    // Only set this the first time the function is called. Knowing the first
    // error message is more useful than the last.
    if (!scnr->error) {
        scnr->error = true;
        scnr->error_message = error_message;
    }
}

// Sometimes there are blocks of code where reaching EOF is an error, setting
// eof_is_error to true will make reaching EOF be an error. This is convenience
// functionality so we don't need to check for EOF every time we call a scanning
// function inside a block like this.
void scanner_eof_set (struct scanner_t *scnr)
{
    scnr->is_eof = true;
    if (scnr->eof_is_error) {
        scanner_set_error (scnr, "Unexpected end of file.");
    }
}

// TODO: I still have to think about parsing optional stuff, sometimes we want
// to test something but not consume it. Maybe split testing and consuming one
// value creating something like scanner_consume_matched() that consumes
// everything matched so far, and something like scanner_reset() that goes back
// to the last position where we consumed something?. Another option is to
// "backup" the position before consuming these things, and if we want to go
// back, restore it (like memory pool markers). I need more experience with the
// API to know which is better for the user, or if there are other alternatives.

bool scanner_float (struct scanner_t *scnr, float *value)
{
    // TODO: Maybe allow value==NULL for the case when we want to consume
    // something but discard it's value.
    assert (value != NULL);
    if (scnr->error)
        return false;

    // Don't accept leading spaces.
    // NOTE: We don't accept floats not starting with a digit like .5, INF or
    // NAN. But we do accept hexadecimal floats like 0x1.Cp2
    if (!isdigit (*scnr->pos)) {
        return false;
    }

    char *end;
    float res = strtof (scnr->pos, &end);
    if (res != 0 || scnr->pos != end) {
        *value = res;
        scnr->pos = end;

        if (*scnr->pos == '\0') {
            scanner_eof_set (scnr);
        }
        return true;
    }

    return false;
}

bool scanner_int (struct scanner_t *scnr, int *value)
{
    // TODO: Maybe allow value==NULL for the case when we want to consume
    // something but discard it's value.
    assert (value != NULL);
    if (scnr->error)
        return false;

    // Don't accept leading spaces.
    if (!isdigit (*scnr->pos)) {
        return false;
    }

    char *end;
    int res = strtol (scnr->pos, &end, 10);
    if (res != 0 || scnr->pos != end) {
        *value = res;
        scnr->pos = end;

        if (*scnr->pos == '\0') {
            scanner_eof_set (scnr);
        }
        return true;
    }

    return false;
}

// NOTE: The definition of a space demends on the locale. In the POSIX locale
// it means space, \n, \f, \r, \t and \v.
// TODO: We should have a scanner_is_space that uses an internal definition of
// what a space is.
void scanner_consume_spaces (struct scanner_t *scnr)
{
    while (isspace(*scnr->pos)) {
        if (*scnr->pos == '\n') {
            scnr->line_number++;
        }
        scnr->pos++;
    }

    if (*scnr->pos == '\0') {
        scanner_eof_set (scnr);
    }
}

bool scanner_char (struct scanner_t *scnr, char c)
{
    if (scnr->error)
        return false;

    if (*scnr->pos == c) {
        scnr->pos++;

        if (*scnr->pos == '\0') {
            scanner_eof_set (scnr);
        }

        return true;
    }

    return false;
}

// TODO: Rename char_any to any_char
bool scanner_char_any (struct scanner_t *scnr, char *char_list)
{
    assert (char_list != NULL);

    if (scnr->error)
        return false;

    while (*char_list != '\0' && *scnr->pos != *char_list) {
        char_list++;
    }

    if (*scnr->pos == '\n') {
        scnr->line_number++;
    }

    if (*char_list != '\0') {
        scnr->pos++;
        return true;
    } else {
        return false;
    }

}

// Consume all characters until c is found. c will be consummed too.
bool scanner_to_char (struct scanner_t *scnr, char c)
{
    if (scnr->error)
        return false;

    while (*scnr->pos != '\0' && *scnr->pos != c) {
        if (*scnr->pos == '\n') {
            scnr->line_number++;
        }
        scnr->pos++;
    }

    if (*scnr->pos == '\0') {
        scanner_eof_set (scnr);
        return false;
    } else {
        scnr->pos++;
        return true;
    }
}

// Consume all characters until any of the characters in char_list is found. The
// found character will be consummed too.
// TODO: Rename char_any to any_char
// NOTE: This has O(n^2) complexity. This is only expected to be used in cases
// where not many characters are expected until a character in char_list is
// found. Also char_list is expected to be small.
bool scanner_to_any_char (struct scanner_t *scnr, char *char_list)
{
    if (scnr->error)
        return false;

    bool found = false;
    while (*scnr->pos != '\0' && !found) {
        char *c = char_list;
        while (*c != '\0') {
            if (*scnr->pos == *c) {
                found = true;
                break;
            }
            c++;
        }

        if (*scnr->pos == '\n') {
            scnr->line_number++;
        }

        scnr->pos++;
    }

    if (*scnr->pos == '\0') {
        scanner_eof_set (scnr);
        return false;
    } else {
        return true;
    }
}

// NOTE: A 'str' containing \n will mess up the line count
bool scanner_str (struct scanner_t *scnr, char *str)
{
    assert (str != NULL);
    if (scnr->error)
        return false;

    size_t len = strlen(str);
    if (strncmp(scnr->pos, str, len) == 0) {
        scnr->pos += len;

        if (*scnr->pos == '\0') {
            scanner_eof_set (scnr);
        }

        return true;
    }

    return false;
}

// NOTE: A 'str' containing \n will mess up the line count
bool scanner_strcase (struct scanner_t *scnr, char *str)
{
    assert (str != NULL);
    if (scnr->error)
        return false;

    size_t len = strlen(str);
    if (strncasecmp(scnr->pos, str, len) == 0) {
        scnr->pos += len;

        if (*scnr->pos == '\0') {
            scanner_eof_set (scnr);
        }

        return true;
    }

    return false;
}


////////////////////////////////////////////////////////////////////////////////
// USE CASE 1
//
// This is the tokenizer that I used for the XKB parser
void xkb_parser_next (struct xkb_parser_state_t *state)
{
    struct scanner_t *scnr = &state->scnr;

    char *identifier_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.";

    scanner_consume_spaces (scnr);
    if (scnr->is_eof) {
        return;
    }

    // Scan out all comments
    while (scanner_str (scnr, "//")) {
        if (scnr->is_eof) {
            xkb_parser_error (state, "Stale '/' character");
        }
        scanner_to_char (scnr, '\n');

        scanner_consume_spaces (scnr);
        if (scnr->is_eof) {
            return;
        }
    }

    char *tok_start;
    if ((tok_start = scnr->pos) && scanner_char_any (scnr, identifier_chars)) {
        state->tok_type = XKB_PARSER_TOKEN_IDENTIFIER;
        while (scanner_char_any (scnr, identifier_chars));
        strn_set (&state->tok_value, tok_start, scnr->pos - tok_start);

        // Check if it's a level identifier, if so, change the type and set the
        // int value. :level_identifiers
        struct scanner_t special_identifier_scnr = ZERO_INIT(struct scanner_t);
        special_identifier_scnr.pos = str_data(&state->tok_value);
        char *bak_start = special_identifier_scnr.pos;
        int level;

        int num_numbers = 0;
        while (scanner_char_any (&special_identifier_scnr, "0123456789")) num_numbers++;

        if (num_numbers == str_len(&state->tok_value)) {
            state->tok_type = XKB_PARSER_TOKEN_NUMBER;
            special_identifier_scnr.pos = bak_start;
            scanner_int (&special_identifier_scnr, &state->tok_value_int);

        } else if ((special_identifier_scnr.pos = bak_start) &&
                   scanner_strcase (&special_identifier_scnr, "level") &&
                   scanner_int (&special_identifier_scnr, &level) &&
                   level > 0 && level <= KEYBOARD_LAYOUT_MAX_LEVELS) {

            state->tok_type = XKB_PARSER_TOKEN_LEVEL_IDENTIFIER;
            state->tok_value_int = level;

        } else if ((special_identifier_scnr.pos = bak_start) &&
                   scanner_strcase (&special_identifier_scnr, "group") &&
                   scanner_int (&special_identifier_scnr, &level) &&
                   level > 0 && level <= KEYBOARD_LAYOUT_MAX_GROUPS) {

            state->tok_type = XKB_PARSER_TOKEN_GROUP_IDENTIFIER;
            state->tok_value_int = level;

        }

    } else if (scanner_char (scnr, '<')) {
        state->tok_type = XKB_PARSER_TOKEN_KEY_IDENTIFIER;

        tok_start = scnr->pos;
        scanner_to_char (scnr, '>');
        if (scnr->is_eof) {
            xkb_parser_error (state, "Key identifier without closing '>'");
        } else {
            strn_set (&state->tok_value, tok_start, scnr->pos - 1 - tok_start);
        }

    } else if (scanner_char_any (scnr, "{}[](),;=+-!")) {
        state->tok_type = XKB_PARSER_TOKEN_OPERATOR;
        strn_set (&state->tok_value, scnr->pos-1, 1);

    } else if (scanner_char (scnr, '\"')) {
        state->tok_type = XKB_PARSER_TOKEN_STRING;

        tok_start = scnr->pos;
        scanner_to_char (scnr, '\"');
        if (scnr->is_eof) {
            xkb_parser_error (state, "String without matching '\"'");
        } else {
            strn_set (&state->tok_value, tok_start, scnr->pos - 1 - tok_start);
        }

    } else {
        xkb_parser_error (state, "Unexpected character %c (0x%x).", *scnr->pos, *scnr->pos);
    }
}

////////////////////////////////////////////////////////////////////////////////
// USE CASE 2
//
// This is a reimplementation of extract_keymap_info() where V1 had been used.
void scan_metadata_value (mem_pool_t *pool, struct scanner_t *scnr, char **val)
{
    // 9 LOC
    assert (val != NULL);

    scanner_consume_spaces(scnr);
    if (scanner_char(scnr, ':')) {
        scanner_consume_spaces (scnr);
        char *start = scnr->pos;
        scanner_to_char (scnr, '\n');
        *val = pom_strndup (pool, start, scnr->pos - start - 1);
    }
}

bool extract_keymap_info (mem_pool_t *pool, char *xkb_file_content, struct keyboard_layout_info_t *info)
{
    // 60 LOC
    if (info == NULL) {
        return false;
    }

    bool success = true;

    *info = ZERO_INIT(struct keyboard_layout_info_t);
    struct scanner_t _scanner = {0};
    struct scanner_t *scnr = &_scanner;
    scnr->pos = xkb_file_content;

    scnr->eof_is_error = true;
    while (scanner_str (scnr, "//")) {
        scanner_consume_spaces (scnr);

        if (scanner_strcase (scnr, "name")) {
            scan_metadata_value (pool,scnr, &info->name);

        } else if (scanner_strcase (scnr, "description")) {
            scan_metadata_value (pool,scnr, &info->description);

        } else if (scanner_strcase (scnr, "short description")) {
            scan_metadata_value (pool,scnr, &info->short_description);

        } else if (scanner_strcase (scnr, "languages")) {
            scanner_consume_spaces(scnr);
            if (scanner_char(scnr, ':')) {
                struct ptrarr_t languages = {0};
                while (*scnr->pos != '\n') {
                    scanner_consume_spaces (scnr);
                    char *start = scnr->pos;
                    scanner_to_any_char (scnr, ",\n");

                    info->num_languages++;
                    char* lang = pom_strndup (pool, start, scnr->pos - start - 1);
                    ptrarr_push (&languages, lang);
                }

                info->languages = pom_push_size (pool, sizeof(char*)*info->num_languages);
                int i;
                for (i=0; i<info->num_languages; i++) {
                    info->languages[i] = languages.data[i];
                }
                ptrarr_free (&languages);
            }
        }

    }
    scnr->eof_is_error = false;

    if (info->name == NULL || info->description == NULL ||
        info->short_description == NULL || info->languages == NULL) {
        success = false;
    }

    return success;
}

