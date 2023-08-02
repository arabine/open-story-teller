#ifndef FS_TASK_H
#define FS_TASK_H

void fs_task_scan_index();
void fs_task_initialize();
void fs_task_load_story(uint8_t *mem);
void fs_task_media_start(char *image, char *sound);

#endif // FS_TASK_H
