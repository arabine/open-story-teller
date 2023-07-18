#ifndef QOR_H
#define QOR_H

#include <stdint.h>
#include <stdbool.h>

extern void ost_hal_panic();

/**
 * Assert, or panic.
 */
#define assert_or_panic(expr) ((expr) ? (void)0U : ost_hal_panic())

/**
 * The module os encapsulates the core functionality of the operating system and
 * exposes the functions for interacting with it.
 */

#define MAXNUMTHREADS 10 /* Maximum number of threads, allocated at compile time */
#define STACKSIZE 100    /* Number of 32-bit words in each TCB's stack */
#define THREADFREQ 1     /* Maximum time-slice, in Hz, before the scheduler is run */

#define OS_SCHEDL_PRIO_MIN 1         /* Lowest priority that can be assigned to a thread */
#define OS_SCHEDL_PRIO_MAX UINT8_MAX /* Highest priority that can be assigned to a thread */

typedef void (*thread_func_t)(void *args);

// ===========================================================================================================
// THREADS API
// ===========================================================================================================

/**
 * @brief RTOS Task state
 *
 */
typedef enum
{
    qor_tcb_state_active,
    qor_tcb_state_sleep
} qor_tcb_state_t;

/**
 * @brief Thread Control Block
 *
 * IMPORTANT! keep the stack pointer on top, it is required by the task switch assembly
 *
 */

typedef struct qor_mbox_t qor_mbox_t;
typedef struct TCB
{
    uint32_t *sp;          //!< Stack pointer, valid for threads not running
    struct TCB *next;      //!< Pointer to circular-linked-list of TCBs
    struct TCB *wait_next; //!< Next TCB in waiting list
    qor_tcb_state_t state; //!< TCB active or free
    uint32_t wait_time;    //!< Timeout for mbox maiting or sleep
    qor_mbox_t *mbox;      //!< Pointer to mailbox on which the thread is blocked, NULL if not blocked
    uint8_t priority;      //!< Thread priority, 0 is highest, 255 is lowest
    uint64_t ts;           //!< system timestamp
    const char *name;      //!< Descriptive name to facilitate debugging

} qor_tcb_t;

void qor_init(uint32_t scheduler_frequency_hz);

void qor_create_thread(qor_tcb_t *tcb, thread_func_t task, uint8_t priority, const char *name);

bool qor_start(qor_tcb_t *idle_tcb, thread_func_t idle_task);

void qor_sleep(uint32_t sleep_duration_ms);

void qor_svc_call(void);

// ===========================================================================================================
// MAILBOX API
// ===========================================================================================================
struct qor_mbox_t
{
    qor_tcb_t *head;
    uint32_t count;
    uint32_t read;
    uint32_t write;
    uint32_t maxCount;
    void **msgBuffer;
};

typedef struct
{
    uint32_t count;
    uint32_t maxCount;
    uint32_t taskCount;
} mbox_stats_t;

#define QOR_MBOX_OK 1
#define QOR_MBOX_ERROR 2
#define QOR_MBOX_FULL 3

void qor_mbox_init(qor_mbox_t *mbox, void **msgBuffer, uint32_t maxCount);
uint32_t qor_mbox_wait(qor_mbox_t *mbox, void **msg, uint32_t wait_ms);

#define QOR_MBOX_OPTION_SEND_FRONT 1
#define QOR_MBOX_OPTION_SEND_BACK 2

uint32_t qor_mbox_notify(qor_mbox_t *mbox, void *msg, uint32_t notifyOption);

#endif // QOR_H
