/**
 * @brief
 *
 * // OST scheduler is a very simple real-time, pre-emptive, tickless tasker
 * Design goals:
 *  - Easily portable (limited assembly)
 *  - Tick-less
 *  - Preemptive
 *  - Everything runs in interrupts
 *  - The background task is calling platform specific sleep modes
 */

#include "ost_hal.h"
#include "debug.h"

typedef struct
{
    uint8_t regs;

} cpu_t;

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
void ost_tasker_timer_callback()
{
    debug_printf("%d\n", counter++);
}

void ost_tasker_schedule(void)
{
}
