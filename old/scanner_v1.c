/*
 * Copiright (C) 2019 Santiago León O.
 */

// This was my first attempt at a scanner API, was useful for little things but
// the code could easily get hairy and it made you constanly think about the
// position of the pointer, where it was returned and if you should update it or
// not. It doesn't have any way to report errors much less to keep track of the
// line number.
//
// Lessons learned from this, doing pointer based parsing eventually gets ugly,
// something like this should be used to create a tokenizer so that higher level
// code is nicer.

////////
// API

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


//////////
// USAGE
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

