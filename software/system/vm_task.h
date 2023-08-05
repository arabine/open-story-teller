#ifndef VM_TASK_H
#define VM_TASK_H

#include <stdint.h>
#include <stdbool.h>

typedef void (*vm_result_cb_t)(bool);

void vm_task_start_story();
void vm_task_initialize();
void vm_task_sound_finished();

#endif // VM_TASK_H
