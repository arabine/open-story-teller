#ifndef QOR_H
#define QOR_H

#define QOR_VERSION "0.2"

#include <stdint.h>
#include <stdbool.h>

extern void ost_hal_panic();

/**
 * Assert, or panic.
 */
#define assert_or_panic(expr) ((expr) ? (void)0U : ost_hal_panic())

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
    qor_tcb_state_active, //!< Thread is active (and can be scheduled or is currently running)
    qor_tcb_state_sleep   //!< Thread is wating for mailbox or sleep
} qor_tcb_state_t;

typedef struct qor_mbox_t qor_mbox_t; //!< Foreward declaration

/**
 * @brief Thread Control Block
 *
 * IMPORTANT! keep the stack pointer on top, it is required by the task switch assembly
 *
 */
typedef struct TCB
{
    uint32_t *sp;          //!< Stack pointer, valid for threads not running, keep it on top
    struct TCB *next;      //!< Pointer to circular-linked-list of TCBs
    struct TCB *wait_next; //!< Next TCB in waiting list
    uint32_t stack_size;   //!< Stack size, in number of uint32_t
    qor_tcb_state_t state; //!< TCB active or free
    uint32_t wait_time;    //!< Timeout for mbox maiting or sleep
    qor_mbox_t *mbox;      //!< Pointer to mailbox on which the thread is blocked, NULL if not blocked
    uint8_t priority;      //!< Thread priority, 0 is highest, 255 is lowest
    uint64_t ts;           //!< system timestamp
    const char *name;      //!< Descriptive name to facilitate debugging

    // Debug/traces
    uint32_t stack_usage;
    uint32_t *stack_bottom;
    bool so; //!< stack overflow detected

} qor_tcb_t;

/**
 * @brief QoRTOS initialization, call it before anything else
 *
 * @param scheduler_frequency_hz: CPU frequency in Hz
 */
void qor_init(uint32_t scheduler_frequency_hz);

/**
 * @brief
 *
 * @param tcb
 * @param task
 * @param stack
 * @param stack_size
 * @param priority
 * @param name
 */
void qor_create_thread(qor_tcb_t *tcb, thread_func_t task, uint32_t *stack, uint32_t stack_size, uint8_t priority, const char *name);

/**
 * @brief
 *
 * @param idle_tcb
 * @param idle_task
 * @param idle_stack
 * @param idle_stack_size
 * @return true
 * @return false
 */
bool qor_start(qor_tcb_t *idle_tcb, thread_func_t idle_task, uint32_t *idle_stack, uint32_t idle_stack_size);

/**
 * @brief
 *
 * @param sleep_duration_ms
 */
void qor_sleep(uint32_t sleep_duration_ms);

// ===========================================================================================================
// MAILBOX API
// ===========================================================================================================
/**
 * @brief
 *
 */
struct qor_mbox_t
{
    qor_tcb_t *head;
    uint32_t count;
    uint32_t read;
    uint32_t write;
    uint32_t maxCount;
    void **msgBuffer;
};

/**
 * @brief
 *
 */
typedef struct
{
    uint32_t count;
    uint32_t maxCount;
    uint32_t taskCount;
} mbox_stats_t;

#define QOR_MBOX_OK 1
#define QOR_MBOX_TIMEOUT 2
#define QOR_MBOX_ERROR 3
#define QOR_MBOX_FULL 4
#define QOR_MBOX_EMPTY 5

/**
 * @brief
 *
 * @param mbox
 * @param msgBuffer
 * @param maxCount
 */
void qor_mbox_init(qor_mbox_t *mbox, void **msgBuffer, uint32_t maxCount);

/**
 * @brief
 *
 * @param mbox
 * @param msg
 * @param wait_ms
 * @return uint32_t
 */
uint32_t qor_mbox_wait(qor_mbox_t *mbox, void **msg, uint32_t wait_ms);

#define QOR_MBOX_OPTION_SEND_FRONT 1
#define QOR_MBOX_OPTION_SEND_BACK 2

/**
 * @brief
 *
 * @param mbox
 * @param msg
 * @param notifyOption
 * @return uint32_t
 */
uint32_t qor_mbox_notify(qor_mbox_t *mbox, void *msg, uint32_t notifyOption);

#endif // QOR_H
