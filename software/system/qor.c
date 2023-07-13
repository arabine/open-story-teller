/**
 * @brief
 *
 * // Quite OK RTOS scheduler is a very simple real-time, pre-emptive, tickless tasker
 * Design goals:
 *  - Easily portable (limited assembly)
 *  - Tick-less
 *  - Preemptive
 *  - Only one inter-thread resource: mailboxes (no mutex, semaphore...)

 */

#include "ost_hal.h"
#include "debug.h"
#include "qor.h"
#include <stdlib.h>

#include "pico/critical_section.h"
#include "hardware/exception.h"

// ===========================================================================================================
// ARM GENERIC
// ===========================================================================================================
inline static void enable_irq()
{
    __asm volatile("cpsie i");
}

inline static void disable_irq()
{
    __asm volatile("cpsid i");
}

void qor_sleep_ms(uint8_t svc, uint32_t ms)
{

    __wfi;
}

static inline void __set_PRIMASK(uint32_t priMask)
{
    __asm volatile("MSR primask, %0"
                   :
                   : "r"(priMask)
                   : "memory");
}

static inline uint32_t __get_PRIMASK(void)
{
    uint32_t result;

    __asm volatile("MRS %0, primask"
                   : "=r"(result));
    return (result);
}

static inline uint32_t qor_enter_critical(void)
{
    uint32_t primask = __get_PRIMASK();
    disable_irq();
    return primask;
}

void qor_exit_critical(uint32_t status)
{
    __set_PRIMASK(status);
}

__attribute__((naked)) void PendSV_Handler()
{
    // __asm("bkpt 1");
    qor_switch_context();
}

__attribute__((optimize("O0"))) static void qor_svc_call(void)
{
    volatile uint32_t *icsr = (void *)0xE000ED04;
    // Pend a PendSV exception using by writing 1 to PENDSVSET at bit 28
    *icsr = 0x1 << 28;
    // flush pipeline to ensure exception takes effect before we
    // return from this routine
    __asm("isb");
}

static const bool qor_inside_interrupt(void)
{
    uint32_t ulCurrentInterrupt;
    bool xReturn;

    /* Obtain the number of the currently executing interrupt. */
    __asm volatile("mrs %0, ipsr"
                   : "=r"(ulCurrentInterrupt)::"memory");

    if (ulCurrentInterrupt == 0)
    {
        xReturn = false;
    }
    else
    {
        xReturn = true;
    }

    return xReturn;
}

// ===========================================================================================================
// GLOBAL AND STATIC VARIABLES
// ===========================================================================================================

static uint32_t Stacks[MAXNUMTHREADS][STACKSIZE];

/* Pointer to the currently running thread */
qor_tcb_t *RunPt = NULL;
static qor_tcb_t *TcbHead = NULL;

/* The variable ActiveTCBsCount tracks the number of TCBs in use by the OS */
static uint32_t ActiveTCBsCount = 0;

// ===========================================================================================================
// Quite Ok RTOS private and public functions
// ===========================================================================================================
void OS_Init(uint32_t scheduler_frequency_hz)
{
    exception_set_exclusive_handler(PENDSV_EXCEPTION, PendSV_Handler);
}

void OS_ExitLoop()
{
    for (;;)
        ;
}

extern void qor_go();

uint32_t *qor_initialize_stack(uint32_t *top_of_stack, thread_func_t task, void *args)
{
    // ARM  Calling convention: the folowwing registers are automatically saved onto the stack by the processor (in this ordoer on the stack)
    // DDI0419C_arm_architecture_v6m_reference_manual-1.pdf  B1.5.6 Exception entry behavior
    top_of_stack--;
    /* From the "STM32 Cortex-M4 Programming Manual" on page 23:
     * attempting to execute instructions when  the T bit is 0 results in a fault or lockup */
    *top_of_stack = 0x01000000; /* Thumb Bit (PSR) */
    top_of_stack--;
    *top_of_stack = (uint32_t)task; // PC Program Counter (R15)
    top_of_stack--;
    *top_of_stack = (uint32_t)OS_ExitLoop; /* (LR) Link Register (Return address) R14 */
    top_of_stack -= 5;                     // skip R12, R3, R2, R1
    *top_of_stack = (uint32_t)args;        // R0
    top_of_stack -= 8;                     // R11 -> R4
    return top_of_stack;
}

void qor_create_thread(qor_tcb_t *tcb, thread_func_t task, uint8_t priority, const char *name)
{
    assert_or_panic(ActiveTCBsCount >= 0 && ActiveTCBsCount < MAXNUMTHREADS);
    disable_irq();

    tcb->state = qor_tcb_state_active;
    tcb->wait_time = 0;
    tcb->state = qor_tcb_state_active;
    tcb->priority = priority;
    tcb->name = name;
    tcb->next = NULL;
    tcb->mbox = NULL;
    tcb->message = NULL;
    tcb->wait_next = NULL;
    tcb->sp = qor_initialize_stack(&Stacks[ActiveTCBsCount][STACKSIZE], task, (void *)name);

    if (TcbHead == NULL)
    {
        TcbHead = tcb;
    }
    else
    {
        // Go to the end of the queue
        qor_tcb_t *t = TcbHead;
        while (t->next != NULL)
        {
            t = t->next;
        }
        // Add TCB to the end of the queue
        t->next = tcb;
    }
    ActiveTCBsCount++;

    enable_irq();
}

void qor_start(void)
{
    assert_or_panic(ActiveTCBsCount > 0);

    // FIXME: use the scheduler to find the best first thread to start
    RunPt = TcbHead;
    /* Prevent the timer's ISR from firing before OSAsm_Start is called */
    disable_irq();

    qor_go();

    /* This statement should not be reached */
    ost_hal_panic();
}

void qor_scheduler(void)
{
    qor_tcb_t *t = TcbHead;

    /* Search for highest priority thread not sleeping or blocked */
    uint32_t max_priority = RunPt->priority;
    qor_tcb_t *best_pt = RunPt;
    while (t != NULL)
    {
        if ((t->priority > max_priority) &&
            (t->wait_time == 0) &&
            (t->state == qor_tcb_state_active))
        {
            best_pt = t;
            max_priority = t->priority;
        }
        t = t->next;
    }

    RunPt = best_pt;
}

void OS_Thread_Suspend(void)
{
    // SchedlTimer_ResetCounter();
}

void qor_sleep(uint32_t sleep_duration_ms)
{
    // RunPt->sleep = sleep_duration_ms;
    // OS_Thread_Suspend();
}

// void OS_DecrementTCBsSleepDuration(void)
// {
//     for (size_t tcb_idx = 0; tcb_idx < MAXNUMTHREADS; tcb_idx++)
//     {
//         if (TCBs[tcb_idx].sleep > 0)
//         {
//             TCBs[tcb_idx].sleep -= 1;
//         }
//     }
// }

void OS_Thread_Kill(void)
{
    /*
    assert_or_panic(ActiveTCBsCount > 1);
    disable_irq();

    qor_tcb_t *previous_tcb = RunPt;
    while (1)
    {
        previous_tcb = previous_tcb->next;
        if (previous_tcb->next == RunPt)
            break;
    }
    qor_tcb_t *next_tcb = RunPt->next;

    previous_tcb->next = next_tcb;
    RunPt->state = qor_tcb_state_free;

    ActiveTCBsCount--;
    enable_irq();
    OS_Thread_Suspend();
    */
}

// ===========================================================================================================
// MAILBOX IMPLEMENTATION
// ===========================================================================================================

#define qor_mbox_tSendNormal 0x0
#define qor_mbox_tSendFront 0x1

void qor_mbox_init(qor_mbox_t *mbox, void **msgBuffer, uint32_t maxCount)
{
    mbox->msgBuffer = msgBuffer;
    mbox->maxCount = maxCount;
    mbox->read = 0;
    mbox->read = 0;
    mbox->head = NULL;
}

uint32_t qor_mbox_wait(qor_mbox_t *mbox, void **msg, uint32_t waitTicks)
{
    uint32_t status = qor_enter_critical();

    if (mbox->count == 0)
    {
        if (waitTicks > 0)
        {
            // No any data, block on that resource
            RunPt->mbox = mbox;
            RunPt->state = qor_tcb_state_wait_mbox;
            RunPt->wait_time = waitTicks;
            qor_exit_critical(status);
            qor_svc_call(); // call scheduler

            status = qor_enter_critical();
            if ((RunPt->wait_time >= 0) && (RunPt->message != NULL))
            {
                *msg = RunPt->message;
                qor_exit_critical(status);
                return QOR_MBOX_OK;
            }
            else
            {
                qor_exit_critical(status);
                return QOR_MBOX_ERROR;
            }
        }
        else
        {
            return QOR_MBOX_ERROR;
        }
    }
    else
    {
        if ((RunPt->message != NULL))
        {
            *msg = RunPt->message;
            qor_exit_critical(status);
            return QOR_MBOX_OK;
        }
        else
        {
            qor_exit_critical(status);
            return QOR_MBOX_ERROR;
        }
    }
}

/*
uint32_t qor_mbox_get(qor_mbox_t *mbox, void **msg)
{
    uint32_t status = qor_enter_critical();
    if (mbox->count > 0)
    {
        --mbox->count;
        *msg = mbox->msgBuffer[mbox->read++];
        if (mbox->read >= mbox->maxCount)
        {
            mbox->read = 0;
        }
        qor_exit_critical(status);
        return tErrorNoError;
    }
    else
    {
        qor_exit_critical(status);
        return tErrorResourceUnavaliable;
    }
}
*/

/*

uint32_t qor_mbox_notify(qor_mbox_t *mbox, void *msg, uint32_t notifyOption)
{
    uint32_t status = qor_enter_critical();
    if (tEventWaitCount(&mbox->event) > 0)
    {
        tTask *task = tEventWakeUp(&mbox->event, (void *)msg, tErrorNoError);
        if (task->prio < currentTask->prio)
        {
            tTaskSched();
        }
    }
    else
    {
        if (mbox->count >= mbox->maxCount)
        {
            qor_exit_critical(status);
            return tErrorResourceFull;
        }
        if (notifyOption & qor_mbox_tSendFront)
        {
            if (mbox->read <= 0)
            {
                mbox->read = mbox->maxCount - 1;
            }
            else
            {
                --mbox->read;
            }
            mbox->msgBuffer[mbox->read] = msg;
        }
        else
        {
            mbox->msgBuffer[mbox->write++] = msg;
            if (mbox->write >= mbox->maxCount)
            {
                mbox->write = 0;
            }
        }
        mbox->count++;
    }
    qor_exit_critical(status);
    return tErrorNoError;
}
*/

void qor_mbox_flush(qor_mbox_t *mbox)
{
    uint32_t status = qor_enter_critical();
    /*
        if (tEventWaitCount(&mbox->event) == 0)
        {
            mbox->read = 0;
            mbox->write = 0;
            mbox->count = 0;
        }
        */
    qor_exit_critical(status);
}

uint32_t mbox_destroy(qor_mbox_t *mbox)
{
    uint32_t status = qor_enter_critical();
    /*
    uint32_t count = tEventRemoveAll(&mbox->event, (void *)0, tErrorDel);
    qor_exit_critical(status);
    if (count > 0)
    {
        tTaskSched();
    }
    */
    return 0; // count;
}

void qor_mbox_get_stats(qor_mbox_t *mbox, mbox_stats_t *info)
{
    uint32_t status = qor_enter_critical();

    info->count = mbox->count;
    info->maxCount = mbox->maxCount;

    info->taskCount = 0;
    qor_tcb_t *head = mbox->head;
    while (head != NULL)
    {
        info->taskCount++;
        head = head->wait_next;
    }

    qor_exit_critical(status);
}
