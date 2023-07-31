#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

// On regroupe ici au les priorités des différents threads afin d'avoir une vision plus large
#define HMI_TASK_PRIORITY 1
#define VM_TASK_PRIORITY 2
#define FS_TASK_PRIORITY 3 ///< High priority for audio / file system access

typedef struct
{
    uint32_t number_of_stories;
    uint32_t current_story;
    uint32_t index_file_size;
    uint32_t rd; ///!< Read index in the Index file

} ost_context_t;

#endif // SYSTEM_H
