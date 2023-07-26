
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

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "audio_player.h"

void ost_hal_panic()
{
}

// ===========================================================================================================
// SD CARD TASK
// ===========================================================================================================
static qor_tcb_t AudioTcb;
static uint32_t AudioStack[4096];

static qor_mbox_t AudioMailBox;

typedef struct
{
    uint8_t ev;
} ost_audio_event_t;

static ost_audio_event_t wake_up;

ost_audio_event_t audio_queue[10];

static int dbg_state = 0;

// End of DMA transfer callback
static void audio_callback(void)
{
    dbg_state = 1 - dbg_state;
    gpio_put(1, dbg_state);
    qor_mbox_notify(&AudioMailBox, (void **)&wake_up, QOR_MBOX_OPTION_SEND_BACK);
}
#include <time.h>
clock_t clock()
{
    return (clock_t)time_us_64() / 1000;
}

void show_duration(uint32_t millisecondes)
{
    uint32_t minutes, secondes, reste;

    // Calcul des minutes, secondes et millisecondes
    minutes = millisecondes / (60 * 1000);
    reste = millisecondes % (60 * 1000);
    secondes = reste / 1000;
    reste = reste % 1000;

    // Affichage du temps
    debug_printf("Temps : %d minutes, %d secondes, %d millisecondes\r\n", minutes, secondes, reste);
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
        debug_printf("\r\n-------------------------------------------------------\r\nPlaying: out2.wav\r\n");
        clock_t startTime = clock();
        ost_audio_play("out2.wav");

        ost_audio_event_t *e = NULL;

        int isPlaying = 0;
        int count = 0;
        do
        {
            uint32_t res = qor_mbox_wait(&AudioMailBox, (void **)&e, 300); // On devrait recevoir un message toutes les 3ms (durée d'envoi d'un buffer I2S)

            if (res == QOR_MBOX_OK)
            {
                isPlaying = ost_audio_process();
            }

            count++;

        } while (isPlaying);

        ost_audio_stop();
        clock_t endTime = clock();
        uint32_t executionTime = endTime - startTime;

        debug_printf("\r\nPackets: %d\r\n", count);
        show_duration(executionTime);

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
