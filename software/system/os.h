#ifndef OS_H
#define OS_H

void ost_tasker_init();

extern void ost_hal_panic();

/**
 * Assert, or panic.
 */
#define assert_or_panic(expr) ((expr) ? (void)0U : ost_hal_panic())

/**
 * The module os encapsulates the core functionality of the operating system and
 * exposes the functions for interacting with it.
 */

#include <stdint.h>

#define MAXNUMTHREADS 10 /* Maximum number of threads, allocated at compile time */
#define STACKSIZE 100    /* Number of 32-bit words in each TCB's stack */
#define THREADFREQ 1     /* Maximum time-slice, in Hz, before the scheduler is run */

#define OS_SCHEDL_PRIO_MIN 1         /* Lowest priority that can be assigned to a thread */
#define OS_SCHEDL_PRIO_MAX UINT8_MAX /* Highest priority that can be assigned to a thread */

/**
 * The type Semaphore_t abstracts the semaphore's counter.
 * A value of type *Semaphore_t should only be updated through the fn OS_Semaphore_Wait
 * and OS_Semaphore_Signal.
 */
typedef int32_t Semaphore_t;

typedef void (*thread_func_t)(void *args);

/**
 * Function descriptions are provided in os.c
 */

void OS_Init(uint32_t scheduler_frequency_hz);

// void OS_Thread_CreateFirst(thread_func_t task, uint8_t priority, const char *name);

void qor_create_thread(thread_func_t task, uint8_t priority, const char *name);

void qor_switch_context();

void qor_start(void);

void OS_Thread_Suspend(void);

void OS_Thread_Sleep(uint32_t sleep_duration_ms);

void OS_DecrementTCBsSleepDuration(void);

void OS_Thread_Kill(void);

void OS_Semaphore_Wait(Semaphore_t *sem);

void OS_Semaphore_Signal(Semaphore_t *sem);

#endif // OST_TASKER_H
