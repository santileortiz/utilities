/*
 * Copyright (C) 2019 Santiago León O.
 */
#include "boilerplate.c"

////////////////////////////////////////////////////////////////////////////////
// API V1
// 110 LOC
//
// This was my first attempt at a scanner API. As soon as I decided to not try
// to have a const correct API I tried to see how far the idiom of V0 would get
// me. Turns out the code got hairy very easily and the API made you constanly
// think about the position of the pointer: keep track of the variable that has
// the most advanced position and if you should update it or not. This API also
// doesn't have any way to report errors much less to keep track of the line
// number. We actually need a struct to keep track of this state.
//
// Conclusion:
//
// Doing pointer based parsing eventually gets ugly, and we really care about
// storing some state when doing more advanced parsing work, like a tokenizer.
//
// There is still value to this one-off function calls: they are easy to call
// anywhere. At least consume_line() continued to be useful when working with
// the XML library and doing some custom string modifications. Using a V2
// scanner of these one time pointer scanning needs looked like too much
// overhead. I may keep some of these in the final version.
//

static inline
bool is_blank (char *c) {
    return *c == ' ' ||  (*c >= '\t' && *c <= '\r');
}

static inline
char *consume_blanks (char *c)
{
    while (is_blank(c)) {
        c++;
    }
    return c;
}

static inline
char* consume_line (char *c)
{
    while (*c && *c != '\n') {
           c++;
    }

    if (*c) {
        c++;
    }

    return c;
}

// Compares the beginning of s with the null terminated c_str, returns true if
// it matches and false otherwise. If after!=NULL and there is a match after is
// set to point to the character after the match. If the string does not match
// we leafe after unchanged.
static inline
bool consume_str (char *s, char *c_str, char **after)
{
    size_t len = strlen(c_str);
    if (memcmp (s, c_str, len) == 0) {
        if (after != NULL) {
            *after = s+len;
        }
        return true;
    } else {
        return false;
    }
}

// Same as consume_str() but ignores the case of the strings.
static inline
bool consume_case_str (char *s, char *c_str, char **after)
{
    size_t len = strlen(c_str);
    if (strncasecmp (s, c_str, len) == 0) {
        if (after != NULL) {
            *after = s+len;
        }
        return true;
    } else {
        return false;
    }
}

static inline
bool consume_char (char *s, char c, char **after)
{
    if (*s == c) {
        if (after != NULL) {
            *after = s+1;
        }
        return true;
    } else {
        return false;
    }
}

static inline
bool consume_until_str (char *s, char *c_str, char **start, char **end)
{
    char *found = strstr (s, c_str);
    if (found) {
        if (start != NULL) {
            *start = found;
        }

        if (end != NULL) {
            *end = found + strlen(c_str);
        }
        return true;
    } else {
        return false;
    }
}

static inline
bool consume_spaces (char *s, char **after)
{
    if (is_space (s)) {
        while (is_space(s)) {
            s++;
        }

        if (after != NULL) {
            *after = s;
        }

        return true;
    } else {
        return false;
    }
}


////////////////////////////////////////////////////////////////////////////////
// USE CASE 1
//
// This is the code where I used the API above originally.

// Parse a block of the form
// <block_id> ["<block_name>"] {<block_content>};
//
// NULL can be passed to arguments if we are not interested in a specific part
// of the block.
//
// Returns a pointer to the first character of the line after the block, or NULL
// if an error occured.
//
// NOTE: This functon does not allocate anything. Resulting pointers point into
// the given string s.
char* parse_xkb_block (char *s,
                       char **block_id, size_t *block_id_size,
                       char **block_name, size_t *block_name_size,
                       char **block_content, size_t *block_content_size)
{
    bool success = true;
    s = consume_blanks (s);
    char *id = s;
    int id_size = 0;
    while (*s && !is_blank (s)) {
        s++;
        id_size++;
    }

    s = consume_blanks (s);
    char *name = NULL;
    int name_size = 0;
    if (*s == '\"') {
        s++;
        name = s;
        while (*s && *s != '\"') {
            s++;
            name_size++;
        }
        s++;
    }

    s = consume_blanks (s);
    char *content = NULL;
    int content_size = 0;
    if (*s == '{') {
        int brace_cnt = 1;
        s++;
        content = s;
        while (*s) {
            if (*s == '{') {
                brace_cnt++;
            } else if (*s == '}') {
                brace_cnt--;
            }
            

            if (brace_cnt == 0) {
                s++; // consume '}'
                break;
            } else {
                s++;
                content_size++;
            }
        }
    } else {
        success = false;
        printf ("Block with invalid content.\n");
    }

    s = consume_blanks (s);
    if (*s != ';') {
        success = false;
        printf ("Missing ; at the end of block.\n");
    }

    if (*s == '\0') {
        success = false;
        printf ("Unexpected end of file.\n");
    }

    s++; // consume ';'

    if (block_id != NULL) {
        *block_id = id;
    }
    if (block_id_size != NULL) {
        *block_id_size = id_size;
    }

    if (block_name != NULL) {
        *block_name = name;
    }
    if (block_name_size != NULL) {
        *block_name_size = name_size;
    }

    if (block_content != NULL) {
        *block_content = content;
    }
    if (block_content_size != NULL) {
        *block_content_size = content_size;
    }

    if (!success) {
        return NULL;
    } else {
        return consume_line(s);
    }
}

////////////////////////////////////////////////////////////////////////////////
// USE CASE 2
// This parses the following kinds of blocks (including the beginning //)
// Name: my_layout
// Description: Test custom layout
// Short description: su
// Languages: es, us
bool extract_keymap_info (mem_pool_t *pool, char *xkb_file_content, struct keyboard_layout_info_t *res)
{
    // 80 LOC
    if (res == NULL) {
        return false;
    }

    *res = (struct keyboard_layout_info_t){0};
    bool success = true;
    char *s = xkb_file_content;
    while (s && *s) {
        if (consume_str (s, "//", &s)) {
            consume_spaces (s, &s);
            if (consume_case_str (s, "name", &s)) {
                consume_spaces(s, &s);
                if (consume_char(s, ':', &s)) {
                    consume_spaces (s, &s);
                    char *end = consume_line (s);
                    res->name = pom_strndup (pool, s, end - s - 1);
                }

            } else if (consume_case_str (s, "description", &s)) {
                consume_spaces(s, &s);
                if (consume_char(s, ':', &s)) {
                    consume_spaces (s, &s);
                    char *end = consume_line (s);
                    res->description = pom_strndup (pool, s, end - s - 1);
                }

            } else if (consume_case_str (s, "short description", &s)) {
                consume_spaces(s, &s);
                if (consume_char(s, ':', &s)) {
                    consume_spaces (s, &s);
                    char *end = consume_line (s);
                    res->short_description = pom_strndup (pool, s, end - s - 1);
                }

            } else if (consume_case_str (s, "languages", &s)) {
                consume_spaces(s, &s);
                if (consume_char(s, ':', &s)) {
                    struct ptrarr_t languages = {0};
                    while (*s && *s != '\n') {
                        // LESSON: Here the semantics of consume_spaces() make
                        // variable names confusing. start starts not at the
                        // actual start of what we want because there may be
                        // spaces that will be consumed by consume_spaces(),
                        // after consume_spaces() is called start is actually
                        // the start of what we want, but then we set a variable
                        // named end to start which is a bit confusing too. The
                        // reasoning here is that end will be the end of what we
                        // want AFTER the while loop finishes. This scanner API
                        // requires creating a lot of temporary pointer
                        // variables that need to have a name and may end up
                        // causing this confusion.
                        //
                        // :consistent_variable_name_semantics
                        // We want the semantics implied by a variable name to
                        // be always the same through the lifetime of the
                        // variable. An API shpold avoid asking the user to come
                        // up with a lot of variable names each time a function
                        // is called. Also an API that updates a pointer as
                        // opposed to initializing it may be the cause of the
                        // issue.
                        char *start = s;
                        consume_spaces (s, &start);
                        char *end = start;
                        while (*end != ',' && *end != '\n') {
                            end++;
                        }
                        s = end + 1;

                        end--;
                        while (is_space(end)) {
                            end--;
                        }
                        end++;

                        res->num_languages++;
                        char* lang = pom_strndup (pool, start, end - start);
                        ptrarr_push (&languages, lang);
                    }

                    res->languages = pom_push_size (pool, sizeof(char*)*res->num_languages);
                    int i;
                    for (i=0; i<res->num_languages; i++) {
                        res->languages[i] = languages.data[i];
                    }
                    ptrarr_free (&languages);
                }
            }
        } 

        s = consume_line (s);
    }

    if (res->name == NULL || res->description == NULL ||
        res->short_description == NULL || res->languages == NULL) {
        success = false;
    }

    return success;
}
