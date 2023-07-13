
#include "ost_hal.h"
#include "debug.h"
#include "filesystem.h"
#include "picture.h"
#include "qor.h"
#include "rotary-button.h"

#define RUN_TESTS 1

#ifndef RUN_TESTS
int main(void)
{
    // Low level initialization, mainly platform stuff
    // After this call, debug_printf *MUST* be available
    ost_system_initialize();
    debug_printf("\r\n [OST] Starting OpenStoryTeller tests: V%d.%d\r\n", 1, 0);

    // File system access
    filesystem_mount();

    // Display
    ost_display_initialize();
    decompress();

    // Audio

    // Tasker
    ost_tasker_init();

    for (;;)
    {
    }

    return 0;
}
#else

#include "sdcard.h"

const uint16_t tones[3][8] =
    {
        {0xff, 131, 147, 165, 175, 196, 220, 247},
        {0xff, 262, 294, 330, 349, 392, 440, 494},
        {0xff, 524, 988, 660, 698, 784, 880, 988},
};

const uint8_t Happy_birthday[] =
    {
        6, 1, 3, 5, 1, 1, 3, 1, 2, 5, 1, 2, 1, 2, 2, 6, 1, 1, 5, 1, 1, 6, 1, 4, 3, 1, 1,
        5, 1, 1, 6, 1, 1, 5, 1, 2, 3, 1, 2, 1, 1, 1, 6, 0, 1, 5, 1, 1, 3, 1, 1, 2, 1, 4,
        2, 1, 3, 3, 1, 1, 5, 1, 2, 5, 1, 1, 6, 1, 1, 3, 1, 2, 2, 1, 2, 1, 1, 4, 5, 1, 4,
        3, 1, 1, 2, 1, 1, 1, 1, 1, 6, 0, 1, 1, 1, 1, 5, 0, 8, 0};

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "audio_player.h"

void ost_hal_panic()
{
}

extern void qor_sleep();

static qor_mbox_t b;

typedef struct
{
    uint8_t ev;
} ost_event_t;

ost_event_t ev_queue[10];

qor_tcb_t tcb1;
qor_tcb_t tcb2;

void UserTask_0(void *args)
{
    //  InstrumentTriggerPE11_Init();
    //  uint32_t count = 0;

    qor_mbox_init(&b, (void **)&ev_queue, 10);
    while (1)
    {

        ost_hal_gpio_set(OST_GPIO_DEBUG_LED, 1);

        //        qor_sleep();
        ost_event_t *e = NULL;
        qor_mbox_wait(&b, (void **)&e, 3);

        for (int i = 0; i < 65500; i++)
        {
            for (int j = 0; j < 100; j++)
                ;
        }

        //        ost_system_delay_ms(500);
        ost_hal_gpio_set(OST_GPIO_DEBUG_LED, 0);
        //        ost_system_delay_ms(500);

        for (int i = 0; i < 65500; i++)
        {
            for (int j = 0; j < 100; j++)
                ;
        }
    }
}

void UserTask_1(void *args)
{
    while (1)
    {
        for (int i = 0; i < 65500; i++)
        {
            for (int j = 0; j < 100; j++)
                ;
        }
        debug_printf("X\n");
        for (int i = 0; i < 65500; i++)
        {
            for (int j = 0; j < 100; j++)
                ;
        }
    }
}
/*
void UserTask_2(void)
{
    InstrumentTriggerPE13_Init();
    uint32_t count = 0;
    while (1)
    {
        InstrumentTriggerPE13_Toggle();
        count++;
        if (count % 35 == 0)
            OS_Thread_Sleep(4500);
        else
            HAL_Delay(70);
    }
}

void UserTask_3(void)
{
    InstrumentTriggerPE14_Init();
    while (1)
    {
        InstrumentTriggerPE14_Toggle();
        HAL_Delay(60);
    }
}
*/
#include "pico/stdlib.h"

int main()
{
    ost_system_initialize();

    // 1. Test the printf output
    debug_printf("\r\n [OST] Starting OpenStoryTeller tests: V%d.%d\r\n", 1, 0);
    /*
      filesystem_mount();

      picture_show("example.bmp");
  */
    // ost_audio_play("out2.wav");

    OS_Init(THREADFREQ);
    qor_create_thread(&tcb1, UserTask_0, 1, "UserTask_0");
    qor_create_thread(&tcb2, UserTask_1, 2, "UserTask_1");
    // OS_Thread_Create(UserTask_2, OS_SCHEDL_PRIO_MAIN_THREAD, "UserTask_2");
    // OS_Thread_Create(OnboardUserButton_Task, OS_SCHEDL_PRIO_EVENT_THREAD, "OnboardUserButton_Task");
    qor_start();

    for (;;)
    {

        // ost_hal_audio_loop();

        // ost_hal_gpio_set(OST_GPIO_DEBUG_LED, 1);
        // ost_system_delay_ms(1000);
        // ost_hal_gpio_set(OST_GPIO_DEBUG_LED, 0);
        // ost_system_delay_ms(1000);
    }
    return 0;
}
#endif
