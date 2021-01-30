/*
 * Copyright (C) 2019 Santiago León O.
 */

// This file contains functions that I wrote at some point and seemed useful but
// I ended up not using. If they prove to actually be useful then we might move
// them into common.h.

#if !defined(COMMON_UNUSED_H)

off_t get_fd_size (int fd)
{
    assert (fd >= 0);
    off_t stat;

    off_t curr = lseek (fd, 0, SEEK_CUR);
    assert (stat != -1);

    stat = lseek (fd, 0, SEEK_SET);
    printf ("Error: %s \n", strerror(errno));
    assert (stat == 0);

    off_t size = lseek (fd, 0, SEEK_END);
    assert (stat != -1);

    stat = lseek (fd, 0, SEEK_SET);
    assert (stat == curr);

    return size;
}

char* full_fd_read (mem_pool_t *pool, int fd)
{
    assert (fd >= 0);

    bool success = true;

    mem_pool_temp_marker_t mrk;
    if (pool != NULL) {
        mrk = mem_pool_begin_temporary_memory (pool);
    }

    char *loaded_data = NULL;
    off_t size = get_fd_size (fd);
    loaded_data = (char*)pom_push_size (pool, size + 1);

    if (fd != -1) {
        int bytes_read = 0;
        do {
            int status = read (fd, loaded_data+bytes_read, size);
            if (status == -1) {
                success = false;
                printf ("Error reading fd: %d: %s\n", fd, strerror(errno));
                break;
            }
            bytes_read += status;
        } while (success && bytes_read != size);
        loaded_data[size] = '\0';
    }

    char *retval = NULL;
    if (success) {
        retval = loaded_data;
    } else if (loaded_data != NULL) {
        if (pool != NULL) {
            mem_pool_end_temporary_memory (mrk);
        } else {
            free (loaded_data);
        }
    }

    return retval;
}


#define COMMON_UNUSED_H
#endif
