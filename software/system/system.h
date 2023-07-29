#ifndef SYSTEM_H
#define SYSTEM_H

// On regroupe ici au les priorités des différents threads afin d'avoir une vision plus large
#define HMI_TASK_PRIORITY 1
#define VM_TASK_PRIORITY 2
#define FS_TASK_PRIORITY 3 ///< High priority for audio / file system access

#endif // SYSTEM_H
