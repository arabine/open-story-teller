#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdbool.h>
#include "system.h"

bool filesystem_read_index_file(ost_context_t *ctx);
void filesystem_mount();
void filesystem_display_image(const char *filename);
void filesystem_load_rom(uint8_t *mem, const char *filename);
void filesystem_get_story_title(ost_context_t *ctx);

#endif // FILESYSTEM_H
