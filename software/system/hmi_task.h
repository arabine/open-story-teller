#ifndef HMI_TASK_H
#define HMI_TASK_H

#include <stdint.h>

void hmi_task_initialize();

void hmi_task_ost_ready(uint32_t number_of_stories);

#endif // HMI_TASK_H
