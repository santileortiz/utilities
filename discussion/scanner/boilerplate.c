/*
 * Copiright (C) 2019 Santiago León O.
 */

////////////////////////////////////////////////////////////////////////////////
// Probably useless pointer array API?
//
// LESSON: This is used for in extract_keymap_info() the usage code but it isn't
// related to the scanner. Although this usecase could become useful for the
// linked list API design.
struct ptrarr_t {
    void **data;
    size_t len;
    size_t size;
};

bool ptrarr_push (struct ptrarr_t *arr, void *ptr)
{
    if (arr->size == 0) {
        arr->size = 10;
        arr->data = malloc (sizeof(void*)*arr->size);

    } else if (arr->len == arr->size-1) {
        void *tmp = realloc (arr->data, sizeof(void*)*arr->size*2);
        if (tmp == NULL) {
            printf ("Realloc failed\n");
            return false;
        } else {
            arr->data = tmp;
            arr->size *= 2;
        }
    }

    arr->data[arr->len] = ptr;
    arr->len++;
    return true;
}

bool ptrarr_free (struct ptrarr_t *arr)
{
    free (arr->data);
    return true;
}
//----------------------------------------------------------------------------//

struct keyboard_layout_info_t {
    char *name;
    char *short_description;
    char *description;
    char **languages;
    int num_languages;
};

