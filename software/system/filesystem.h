#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdbool.h>

typedef struct
{
    uint32_t number_of_stories;
    uint32_t current_story;
} ost_context_t;

bool filesystem_read_index_file(ost_context_t *ctx);
void filesystem_mount();

#endif // FILESYSTEM_H
