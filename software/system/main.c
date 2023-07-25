
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

// Raspberry Pico SDK
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "pico.h"
#include "pico/stdlib.h"

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

void UserTask_1(void *args)
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

// ===========================================================================================================
// SD CARD TASK
// ===========================================================================================================
static qor_tcb_t AudioTcb;
static uint32_t AudioStack[4096];

static qor_mbox_t AudioMailBox;

static ost_event_t wake_up;

typedef struct
{
    uint8_t ev;
} ost_audio_event_t;

ost_audio_event_t audio_queue[10];

// End of DMA transfer callback
static void audio_callback(void)
{
    qor_mbox_notify(&b, (void **)&wake_up, QOR_MBOX_OPTION_SEND_BACK);
    gpio_xor_mask(1 << 1);
}

void AudioTask(void *args)
{
    picture_show("example.bmp");
    // ost_audio_play("out2.wav");

    wake_up.ev = 34;

    qor_mbox_init(&AudioMailBox, (void **)&audio_queue, 10);

    ost_audio_register_callback(audio_callback);

    gpio_init(1);
    gpio_set_dir(1, GPIO_OUT);

    static bool onetime = true;
    gpio_put(1, 0);

    while (1)
    {

// Benchmark code
#if 0
        if (onetime)
        {
            onetime = false;

            ost_audio_play("out2.wav");

            int isPlaying = 0;
            int count = 0;
            do
            {
                gpio_put(1, 1);
                isPlaying = ost_audio_process();
                gpio_put(1, 0);
                count++;

            } while (isPlaying);
            debug_printf("Packets: %d\r\n", count);
        }
#endif

        ost_audio_play("out2.wav");

        ost_event_t *e = NULL;

        int isPlaying = 0;
        int count = 0;
        do
        {
            uint32_t res = qor_mbox_wait(&AudioMailBox, (void **)&e, 30); // On devrait recevoir un message toutes les 3ms (durée d'envoi d'un buffer I2S)

            if (res == QOR_MBOX_OK)
            {
                isPlaying = ost_audio_process();
            }

            count++;

        } while (isPlaying);

        for (;;)
        {
            ost_hal_gpio_set(OST_GPIO_DEBUG_LED, 0);
            qor_sleep(500);
            ost_hal_gpio_set(OST_GPIO_DEBUG_LED, 1);
            qor_sleep(500);
        }
    }
}

// ===========================================================================================================
// IDLE TASK
// ===========================================================================================================
static qor_tcb_t IdleTcb;
static uint32_t IdleStack[1024];
void IdleTask(void *args)
{
    while (1)
    {
        // Instrumentation, power saving, os functions won't work here
        // __asm volatile("wfi");
    }
}

// ===========================================================================================================
// MAIN ENTRY POINT
// ===========================================================================================================
int main()
{
    // 1. Call the platform initialization
    ost_system_initialize();

    // 2. Test the printf output
    debug_printf("\r\n [OST] Starting OpenStoryTeller tests: V%d.%d\r\n", 1, 0);

    // 3. Filesystem / SDCard initialization
    filesystem_mount();

    // 4. Initialize OS and threads
    qor_init(125000000UL);

    // qor_create_thread(&tcb1, UserTask_1, 2, "UserTask_1");
    // qor_create_thread(&tcb2, UserTask_2, 1, "UserTask_2");
    qor_create_thread(&AudioTcb, AudioTask, AudioStack, sizeof(AudioStack) / sizeof(AudioStack[0]), 3, "AudioTask"); ///< High priority for audio
    qor_start(&IdleTcb, IdleTask, IdleStack, 1024);

    return 0;
}
#endif
