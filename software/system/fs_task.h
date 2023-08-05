#ifndef FS_TASK_H
#define FS_TASK_H

#include <stdint.h>

typedef void (*fs_result_cb_t)(bool);

void fs_task_scan_index(fs_result_cb_t cb);
void fs_task_initialize();
void fs_task_load_story(uint8_t *mem);
void fs_task_image_start(char *image);
void fs_task_sound_start(char *sound);
void fs_task_play_index();
void fs_task_read_block(uint32_t addr, uint8_t *block, fs_result_cb_t cb);

#endif // FS_TASK_H
