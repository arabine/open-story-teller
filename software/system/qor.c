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
#include <string.h>

// Raspberry Pico SDK
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "pico.h"
#include "pico/critical_section.h"
#include "hardware/exception.h"
#include "RP2040.h"

void qor_switch_context();
void qor_go();
// void qor_svc_call(void);

#define portNVIC_INT_CTRL_REG (*((volatile uint32_t *)0xe000ed04))
#define portNVIC_PENDSVSET_BIT (1UL << 28UL)

#define qor_svc_call()                                  \
    do                                                  \
    {                                                   \
        portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT; \
    } while (0)

// ===========================================================================================================
// ARM GENERIC
// ===========================================================================================================

static uint32_t gCritialNesting = 0;

#define enable_irq() __asm volatile("cpsie i")
#define disable_irq() __asm volatile("cpsid i")

/*
static inline uint32_t qor_enter_critical(void)
{
    uint32_t primask = __get_PRIMASK();
    disable_irq();
    gCritialNesting++;
    __asm volatile("dsb" ::
                       : "memory");
    __asm volatile("isb");
    return primask;
}

void qor_exit_critical(uint32_t status)
{
    gCritialNesting--;

    if (gCritialNesting == 0)
    {
        enable_irq();
    }
    __set_PRIMASK(status);
}
*/

static const bool qor_inside_interrupt(void)
{
    uint32_t ulCurrentInterrupt;
    // Obtain the number of the currently executing interrupt
    __asm volatile("mrs %0, ipsr"
                   : "=r"(ulCurrentInterrupt)::"memory");
    return ulCurrentInterrupt == 0 ? false : true;
}

// ===========================================================================================================
// RASPBERRY PICO
// ===========================================================================================================

static volatile uint32_t timer_period;

#define ALARM_NUM 0
#define ALARM_IRQ TIMER_IRQ_0

#include "hardware/structs/systick.h"

//-----------------------------------------------------------------------------
// Beware, do not consume any local variable here, and keep the "naked" attribute
// Since it is an Exception (interrupt), we work on the MSB
// The syscall wil then call the scheduler so we will never free any local variables consumed
// It will result on a MSP infinite growing (0x42000 on the Pico) until user data overriding and at the end an hardfault will occur
// static void timer_irq(void)
// {
//     // Clear the alarm irq
//     hw_clear_bits(&timer_hw->intr, 1u << ALARM_NUM);
//     qor_svc_call();
// }

static uint64_t gNextAlarm = 0;

static void timer_set_alarm(uint32_t delay_ms)
{
    if (delay_ms > 0)
    {
        // Alarm is only 32 bits so if trying to delay more
        // than that need to be careful and keep track of the upper
        // bits
        gNextAlarm = timer_hw->timerawl + delay_ms * 1000;
        timer_hw->alarm[ALARM_NUM] = (uint32_t)gNextAlarm;
        // Enable the interrupt for our alarm (the timer outputs 4 alarm irqs)
        hw_set_bits(&timer_hw->inte, 1u << ALARM_NUM);
    }
}

//  __attribute__((naked))
static void timer_end()
{
    timer_hw->armed = 0xF;
    // Clear the alarm irq
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_NUM);
    // Disable the intterupt for this alarm
    hw_clear_bits(&timer_hw->inte, 1u << ALARM_NUM);

    qor_svc_call();
}

//-----------------------------------------------------------------------------
static void timer_init()
{
    // Set irq handler for alarm irq
    irq_set_exclusive_handler(ALARM_IRQ, timer_end);
    // Disable the intterupt for this alarm
    hw_clear_bits(&timer_hw->inte, 1u << ALARM_NUM);
    // Enable the alarm irq
    irq_set_enabled(ALARM_IRQ, true);
}

// ===========================================================================================================
// GLOBAL AND STATIC VARIABLES
// ===========================================================================================================
/* Pointer to the currently running thread */
qor_tcb_t *RunPt = NULL;
static qor_tcb_t *TcbHead = NULL;
static qor_tcb_t *IdleTcb = NULL;
static thread_func_t IdleTask = NULL;

/* The variable ActiveTCBsCount tracks the number of TCBs in use by the OS */
static uint32_t ActiveTCBsCount = 0;

// ===========================================================================================================
// Quite Ok RTOS private and public functions
// ===========================================================================================================
// void qor_svc_call(void)
// {
//     //  hw_clear_bits(&timer_hw->inte, 1u << ALARM_NUM);
//     // volatile uint32_t *icsr = (void *)0xE000ED04;
//     // // Pend a PendSV exception using by writing 1 to PENDSVSET at bit 28
//     // *icsr = 0x1 << 28;
//     // // flush pipeline to ensure exception takes effect before we
//     // // return from this routine
//     //
//     // __asm volatile("svc 0");
// }

void __attribute__((naked)) swc()
{
    qor_switch_context();
}

void qor_init(uint32_t scheduler_frequency_hz)
{
    exception_set_exclusive_handler(PENDSV_EXCEPTION, qor_switch_context);
}

static void qor_exit_loop()
{
    for (;;)
        ;
}

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
    *top_of_stack = (uint32_t)qor_exit_loop; /* (LR) Link Register (Return address) R14 */
    top_of_stack -= 5;                       // skip R12, R3, R2, R1
    *top_of_stack = (uint32_t)args;          // R0
    top_of_stack -= 8;                       // R11 -> R4
    return top_of_stack;
}

void qor_create_thread(qor_tcb_t *tcb, thread_func_t task, uint32_t *stack, uint32_t stack_size, uint8_t priority, const char *name)
{
    // assert_or_panic(ActiveTCBsCount >= 0 && ActiveTCBsCount < MAXNUMTHREADS);
    disable_irq();

    memset(&stack[0], 0xAAAAAAAA, sizeof(stack[0]) * stack_size);

    tcb->stack_bottom = &stack[0];
    tcb->stack_size = stack_size;
    tcb->stack_usage = 0;
    tcb->so = false;

    tcb->state = qor_tcb_state_active;
    tcb->wait_time = 0;
    tcb->priority = priority;
    tcb->name = name;
    tcb->next = NULL;
    tcb->mbox = NULL;
    tcb->wait_next = NULL;
    // The ARM architecture is full-descending task, so we indicate the last occupied entry (not a free cell)
    tcb->sp = qor_initialize_stack(&stack[stack_size], task, (void *)name);

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

bool __attribute__((naked)) qor_start(qor_tcb_t *idle_tcb, thread_func_t idle_task, uint32_t *idle_stack, uint32_t idle_stack_size)
{
    assert_or_panic(ActiveTCBsCount > 0);

    if ((idle_task == NULL) || (idle_tcb == NULL))
    {
        return false;
    }

    qor_create_thread(idle_tcb, idle_task, idle_stack, idle_stack_size, 0, "IdleTask");

    IdleTcb = idle_tcb;
    IdleTask = idle_task;
    RunPt = IdleTcb;

    // Find the best first thread to start (highest priority)
    qor_tcb_t *t = TcbHead;
    if (t != NULL)
    {
        while (t->next != NULL)
        {
            if (t->priority > RunPt->priority)
            {
                RunPt = t;
            }
            t = t->next;
        }
    }

    timer_init();

    /* Prevent the timer's ISR from firing before start is called */
    enable_irq();
    qor_go();

    /* This statement should not be reached */
    ost_hal_panic();

    return true;
}

// void __not_in_flash_func(qor_scheduler)(void)
void qor_scheduler(void)
{
    /*
        La stratégie est la suivante:
        - On va circuler parmi tous les TCB (liste chaînée)
        - On va retenir le TCB dont la priorité est la plus élevée parmi les actifs
        - On va retenir le TCB dont la priorité est la plus élevée parmi les endormis
        - On va élir le TCB actif trouvé, sinon l'endormi, et finalement la tâche idle si aucune autre tâche n'est éligible
     */
    uint32_t max_priority = 0;
    qor_tcb_t *best_active = NULL;
    qor_tcb_t *best_sleeping = NULL;
    qor_tcb_t *t = TcbHead;

    uint32_t next_alarm = 60000; // default alarm is next is 60 seconds

    uint64_t ts = time_us_64() / 1000; // in ms

    while (t != NULL)
    {
        uint64_t final = t->ts + t->wait_time;

        // First look if the task can be woken-up
        if (final <= ts)
        {
            t->wait_time = 0;
        }

        if ((t->priority > max_priority) &&
            (t->wait_time == 0))
        {
            max_priority = t->priority;
            if (t->state == qor_tcb_state_active)
            {
                best_active = t;
            }
            else
            {
                best_sleeping = t;
            }
        }

        // Compute the minimal alarm delay asked
        if (t->wait_time > 0)
        {
            // We have a sleep order, compute distance to final absolute timestamp
            uint64_t diff = final - ts;
            if (diff < next_alarm)
            {
                next_alarm = diff;
            }
        }

        t = t->next;
    }

    if (best_active != NULL)
    {
        RunPt = best_active;
    }
    else if (best_sleeping != NULL)
    {
        // On va réveiller un endormi, car son temps d'attente est dépassé (ok ou timeout, ça dépend si c'est un sleep volontaire ou attente de mailbox dépassée)
        RunPt = best_sleeping;
        RunPt->state = qor_tcb_state_active; // devient actif
        RunPt->ts = 0;                       // means timeout
    }
    else
    {
        RunPt = IdleTcb;
    }

    // Statistics
    if (RunPt->stack_bottom[0] != 0xAAAAAAAA)
    {
        RunPt->so = true;
    }
    RunPt->stack_usage = RunPt->sp - RunPt->stack_bottom;

    timer_set_alarm(next_alarm);
}

void qor_sleep(uint32_t sleep_duration_ms)
{
    if (sleep_duration_ms > 0)
    {
        disable_irq();
        RunPt->state = qor_tcb_state_sleep;
        RunPt->ts = time_us_64() / 1000;
        RunPt->wait_time = sleep_duration_ms;
        enable_irq();
        qor_svc_call(); // call scheduler, recompute next timeout
    }
}

// ===========================================================================================================
// MAILBOX IMPLEMENTATION
// ===========================================================================================================

void qor_mbox_init(qor_mbox_t *mbox, void **msgBuffer, uint32_t maxCount)
{
    mbox->msgBuffer = msgBuffer;
    mbox->maxCount = maxCount;
    mbox->read = 0;
    mbox->head = NULL;
    mbox->count = 0;
}

uint32_t qor_mbox_wait(qor_mbox_t *mbox, void **msg, uint32_t wait_ms)
{
    disable_irq();

    // No any data, block on that resource
    if (mbox->count == 0)
    {
        if (wait_ms > 0)
        {
            RunPt->mbox = mbox;
            mbox->head = RunPt;
            qor_sleep(wait_ms);

            disable_irq();
            if (RunPt->ts == 0)
            {
                enable_irq();
                return QOR_MBOX_TIMEOUT;
            }
        }
        else
        {
            enable_irq();
            return QOR_MBOX_ERROR;
        }
    }

    --mbox->count;
    *msg = mbox->msgBuffer[mbox->read++];
    if (mbox->read >= mbox->maxCount)
    {
        mbox->read = 0;
    }
    enable_irq();
    return QOR_MBOX_OK;
}

uint32_t qor_mbox_notify(qor_mbox_t *mbox, void *msg, uint32_t notifyOption)
{
    disable_irq();

    if (mbox->count >= mbox->maxCount)
    {
        enable_irq();
        return QOR_MBOX_FULL;
    }
    if (notifyOption == QOR_MBOX_OPTION_SEND_FRONT)
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

    // We warn waiting thread that a new message is available
    qor_tcb_t *t = mbox->head;
    if (t != NULL)
    {
        t->wait_time = 0;
        t->state = qor_tcb_state_active; // force wake up
    }

    enable_irq();
    qor_svc_call(); // call scheduler
    return QOR_MBOX_OK;
}

void qor_mbox_get_stats(qor_mbox_t *mbox, mbox_stats_t *info)
{
    disable_irq();

    info->count = mbox->count;
    info->maxCount = mbox->maxCount;

    info->taskCount = 0;
    qor_tcb_t *head = mbox->head;
    while (head != NULL)
    {
        info->taskCount++;
        head = head->wait_next;
    }

    enable_irq();
}
