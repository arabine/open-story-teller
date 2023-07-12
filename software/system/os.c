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
#include "os.h"
#include <stdlib.h>

#include "pico/critical_section.h"

/**
 * The fn OSAsm_ThreadSwitch, implemented in os_asm.s, is periodically called by the SchedlTimer (ISR).
 * It preemptively switches to the next thread, that is, it stores the stack of the running
 * thread and restores the stack of the next thread.
 * It calls OS_Schedule to determine which thread is run next and update RunPt.
 */
extern void OSAsm_ThreadSwitch(void);

static critical_section_t acrit;

inline static void enable_irq()
{
    critical_section_exit(&acrit);
}

inline static void disable_irq()
{
    critical_section_enter_blocking(&acrit);
}

void timer_set_period(uint16_t i);

void ost_tasker_sleep_for(uint32_t ms)
{
    timer_set_period(ms);
}

void ost_tasker_init()
{
    ost_tasker_sleep_for(5000); // 5 seconds
}

static uint32_t counter = 0;

// void ost_tasker_timer_callback()
// {
//     // debug_printf("%d\n", counter++);

//     qor_switch_context();
// }

#define OS_DEBUG

/**
 * TCBState indicates whether the TCB can be used by OS_ThreadCreate
 * to create a new thread.
 */
typedef enum
{
    TCBStateFree,
    TCBStateActive
} TCBState_t;

void qor_sleep_ms(uint8_t svc, uint32_t ms)
{

    __wfi;
}

/**
 * Thread Control Block
 *
 * IMPORTANT!
 * The fn OSAsm_Start and OSAsm_ThreadSwitch, implemented in os_asm.s, expect the stack pointer
 * to be placed first in the struct. Don't shuffle it!
 */
typedef struct TCB
{
    uint32_t *sp;         /* Stack pointer, valid for threads not running */
    struct TCB *next;     /* Pointer to circular-linked-list of TCBs */
    uint32_t sleep;       /* Sleep duration in ms, zero means not sleeping */
    TCBState_t status;    /* TCB active or free */
    Semaphore_t *blocked; /* Pointer to semaphore on which the thread is blocked, NULL if not blocked */
    uint8_t priority;     /* Thread priority, 0 is highest, 255 is lowest */
    const char *name;     /* Descriptive name to facilitate debugging */
    uint32_t pc;
} TCB_t;

//==================================================================================================
// GLOBAL AND STATIC VARIABLES
//==================================================================================================

static TCB_t TCBs[MAXNUMTHREADS];
static uint32_t Stacks[MAXNUMTHREADS][STACKSIZE];

/* Pointer to the currently running thread */
TCB_t *RunPt;

static uint32_t global_stack[1000];

/* The variable ActiveTCBsCount tracks the number of TCBs in use by the OS */
static uint32_t ActiveTCBsCount;

static void OS_InitTCBsStatus(void)
{
    for (uint32_t idx = 0; idx < MAXNUMTHREADS; idx++)
    {
        TCBs[idx].status = TCBStateFree;
    }
}

void OS_Init(uint32_t scheduler_frequency_hz)
{
    critical_section_init(&acrit);
    OS_InitTCBsStatus();
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

void qor_create_thread(thread_func_t task, uint8_t priority, const char *name)
{
    assert_or_panic(ActiveTCBsCount >= 0 && ActiveTCBsCount < MAXNUMTHREADS);
    disable_irq();

    /* Find next available TCB */
    int32_t new_tcb_idx = -1;
    for (new_tcb_idx = 0; new_tcb_idx < MAXNUMTHREADS; new_tcb_idx++)
    {
        if (TCBs[new_tcb_idx].status == TCBStateFree)
        {
            break;
        }
    }

    if (new_tcb_idx >= 0)
    {
        if (new_tcb_idx == 0)
        {
            RunPt = &(TCBs[0]);
        }
        else
        {
            TCBs[new_tcb_idx].next = RunPt->next;
        }

        TCBs[new_tcb_idx].sleep = 0;
        TCBs[new_tcb_idx].status = TCBStateActive;
        TCBs[new_tcb_idx].blocked = NULL;
        TCBs[new_tcb_idx].priority = priority;
        TCBs[new_tcb_idx].name = name;
        TCBs[new_tcb_idx].sp = qor_initialize_stack(&Stacks[new_tcb_idx][STACKSIZE], task, (void *)name);

        RunPt->next = &(TCBs[new_tcb_idx]);
        ActiveTCBsCount++;
    }

    enable_irq();
}

void qor_start(void)
{
    assert_or_panic(ActiveTCBsCount > 0);

    /* Prevent the timer's ISR from firing before OSAsm_Start is called */
    disable_irq();

    qor_go();

    /* This statement should not be reached */
    ost_hal_panic();
}

void qor_scheduler(void)
{
    TCB_t *next_pt = RunPt->next;
    TCB_t *iterating_pt = next_pt;

    /* Search for highest priority thread not sleeping or blocked */
    uint32_t max_priority = RunPt->priority;
    TCB_t *best_pt = next_pt;
    do
    {
        if ((iterating_pt->priority > max_priority) && (iterating_pt->sleep == 0) && (iterating_pt->blocked == NULL))
        {
            best_pt = iterating_pt;
            max_priority = best_pt->priority;
        }
        iterating_pt = iterating_pt->next;
    } while (iterating_pt != next_pt);

    RunPt = best_pt;
}

void OS_Thread_Suspend(void)
{
    // SchedlTimer_ResetCounter();
}

void OS_Thread_Sleep(uint32_t sleep_duration_ms)
{
    RunPt->sleep = sleep_duration_ms;
    OS_Thread_Suspend();
}

void OS_DecrementTCBsSleepDuration(void)
{
    for (size_t tcb_idx = 0; tcb_idx < MAXNUMTHREADS; tcb_idx++)
    {
        if (TCBs[tcb_idx].sleep > 0)
        {
            TCBs[tcb_idx].sleep -= 1;
        }
    }
}

void OS_Thread_Kill(void)
{
    assert_or_panic(ActiveTCBsCount > 1);
    disable_irq();

    TCB_t *previous_tcb = RunPt;
    while (1)
    {
        previous_tcb = previous_tcb->next;
        if (previous_tcb->next == RunPt)
            break;
    }
    TCB_t *next_tcb = RunPt->next;

    previous_tcb->next = next_tcb;
    RunPt->status = TCBStateFree;

    ActiveTCBsCount--;
    enable_irq();
    OS_Thread_Suspend();
}

void OS_Semaphore_Wait(Semaphore_t *sem)
{
    disable_irq();
    (*sem) = (*sem) - 1;
    if ((*sem) < 0)
    {
        RunPt->blocked = sem; /* Reason the thread is blocked */
        enable_irq();
        OS_Thread_Suspend();
    }
    enable_irq();
}

void OS_Semaphore_Signal(Semaphore_t *sem)
{
    disable_irq();
    (*sem) = (*sem) + 1;
    if ((*sem) <= 0)
    {
        /* Search for a TCB blocked on this semaphore and wake it up */
        TCB_t *a_tcb = RunPt->next;
        while (a_tcb->blocked != sem)
        {
            a_tcb = a_tcb->next;
        }
        a_tcb->blocked = 0;
    }
    enable_irq();
}
