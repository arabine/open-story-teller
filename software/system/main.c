
#include "ost_hal.h"
#include "debug.h"
#include "filesystem.h"
#include "picture.h"
#include "qor.h"
#include "rotary-button.h"

#include "sdcard.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "audio_player.h"
#include "chip32_vm.h"
#include "mini_qoi.h"

#ifdef OST_USE_FF_LIBRARY
#include "ff.h"
#include "diskio.h"
typedef FIL file_t;
#else

// Use standard library
typedef FILE *file_t;
typedef int FRESULT;
#define F_OK
#endif

file_t file_open(const char *filename)
{
#ifdef OST_USE_FF_LIBRARY
    file_t fil;
    FRESULT fr = f_open(&fil, filename, FA_READ);
    if (fr != FR_OK)
    {
        debug_printf("ERROR: f_open %d\n\r", (int)fr);
    }
    return fil;
#else
    return fopen(filename, "r");
#endif
}

void ost_hal_panic()
{
}

// ===========================================================================================================
// GLOBAL STORY VARIABLES
// ===========================================================================================================
static ost_context_t OstContext;

static mqoi_dec_t dec;
static mqoi_desc_t desc;
/*
if (pixel == info_header.width)
{
    // enough pixels to write a line to the screen
    ost_display_draw_h_line(pos.y, decompressed, palette);
    // debug_printf("POS Y: %d", pos.y);

    memset(decompressed, 0, sizeof(decompressed));
    // ili9341_write(&pos, decompressed);
    // next line...
    pos.y++;
    totalPixels += info_header.width;
    pixel = 0;
    nblines++;
}
*/

static uint8_t bmpImage[256];

static color_t line[320];

void display_image(const char *filename)
{
    file_t fil;
    unsigned int br;

    fil = file_open(filename);
    uint32_t imgW, imgH;

    mqoi_desc_init(&desc);

    // 1. Read header
    f_read(&fil, &desc.magic[0], sizeof(mqoi_desc_t) - 1, &br);

    uint8_t errn = mqoi_desc_verify(&desc, &imgW, &imgH);

    if (errn)
    {
        debug_printf("Invalid image, code %d\n", errn);
        return;
    }

    debug_printf("Image dimensions: %d, %d\n", imgW, imgH);

    uint32_t start, end, pxCount = 0;

    volatile mqoi_rgba_t *px;

    mqoi_dec_init(&dec, imgW * imgH);

    // Serial.println("starting decode...");
    int index = 256; // force refill first time
    int x = 0;
    int y = 0;
    while (!mqoi_dec_done(&dec))
    {
        if (index >= sizeof(bmpImage))
        {
            // refill buffer
            f_read(&fil, bmpImage, sizeof(bmpImage), &br);
            index = 0;
        }

        mqoi_dec_push(&dec, bmpImage[index++]);

        while ((px = mqoi_dec_pop(&dec)) != NULL)
        {
            pxCount++;

            line[x].r = px->r;
            line[x].g = px->g;
            line[x].b = px->b;

            x++;
            if (x >= 320)
            {
                ost_display_draw_h_line_rgb888(y, line);
                x = 0;
                y++;
            }
        }
    }

    f_close(&fil);
}

// ===========================================================================================================
// HMI TASK (user interface, buttons manager, LCD)
// ===========================================================================================================
static qor_tcb_t HmiTcb;
static uint32_t HmiStack[4096];

static qor_mbox_t HmiMailBox;

typedef struct
{
    uint8_t ev;
} ost_hmi_event_t;

static ost_hmi_event_t HmiEvent;

ost_hmi_event_t HmiQueue[10];

void HmiTask(void *args)
{
    qor_mbox_init(&HmiMailBox, (void **)&HmiQueue, 10);

    ost_hmi_event_t *e = NULL;

    filesystem_read_index_file(&OstContext);

    display_image("/ba869e4b-03d6-4249-9202-85b4cec767a7/images/bird.qoi");

    while (1)
    {
        uint32_t res = qor_mbox_wait(&HmiMailBox, (void **)&e, 1000);

        if (res == QOR_MBOX_OK)
        {
        }
        else
        {
            debug_printf("H"); // pour le debug only
        }
    }
}

// ===========================================================================================================
// VIRTUAL MACHINE TASK
// ===========================================================================================================
static qor_tcb_t VmTcb;
static uint32_t VmStack[4096];

static qor_mbox_t VmMailBox;

typedef struct
{
    uint8_t ev;
} ost_vm_event_t;
ost_vm_event_t VmQueue[10];

static ost_vm_event_t VmEvent;

static uint8_t m_rom_data[16 * 1024];
static uint8_t m_ram_data[16 * 1024];
static chip32_ctx_t m_chip32_ctx;

// Index file parameter, reference an index in the file

uint8_t vm_syscall(chip32_ctx_t *ctx, uint8_t signum)
{
}

void VmTask(void *args)
{
    // VM Initialize
    m_chip32_ctx.stack_size = 512;

    m_chip32_ctx.rom.mem = m_rom_data;
    m_chip32_ctx.rom.addr = 0;
    m_chip32_ctx.rom.size = sizeof(m_rom_data);

    m_chip32_ctx.ram.mem = m_ram_data;
    m_chip32_ctx.ram.addr = sizeof(m_rom_data);
    m_chip32_ctx.ram.size = sizeof(m_ram_data);

    m_chip32_ctx.syscall = vm_syscall;

    chip32_initialize(&m_chip32_ctx);

    qor_mbox_init(&VmMailBox, (void **)&VmQueue, 10);

    chip32_result_t run_result;
    ost_vm_event_t *e = NULL;

    while (1)
    {
        uint32_t res = qor_mbox_wait(&VmMailBox, (void **)&e, 300); // On devrait recevoir un message toutes les 3ms (durée d'envoi d'un buffer I2S)

        if (res == QOR_MBOX_OK)
        {
            if (VmEvent.ev == 1)
            {
                do
                {
                    run_result = chip32_step(&m_chip32_ctx);
                } while (run_result != VM_OK);
            }
        }
    }
}

// ===========================================================================================================
// AUDIO TASK
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
    qor_mbox_notify(&AudioMailBox, (void **)&wake_up, QOR_MBOX_OPTION_SEND_BACK);
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
    // picture_show("example.bmp");

    wake_up.ev = 34;

    qor_mbox_init(&AudioMailBox, (void **)&audio_queue, 10);

    ost_audio_register_callback(audio_callback);

    static bool onetime = true;

    while (1)
    {
        debug_printf("\r\n-------------------------------------------------------\r\nPlaying: out2.wav\r\n");
        ost_system_stopwatch_start();
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

        uint32_t executionTime = ost_system_stopwatch_stop();
        ost_audio_stop();

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

    // 4. Initialize OS before all other OS calls
    qor_init(125000000UL);

    // 5. Initialize the tasks
    qor_create_thread(&HmiTcb, HmiTask, HmiStack, sizeof(HmiStack) / sizeof(HmiStack[0]), 1, "HmiTask"); // less priority is the HMI (user inputs and LCD)
    qor_create_thread(&VmTcb, VmTask, VmStack, sizeof(VmStack) / sizeof(VmStack[0]), 2, "VmTask");
    qor_create_thread(&AudioTcb, AudioTask, AudioStack, sizeof(AudioStack) / sizeof(AudioStack[0]), 3, "AudioTask"); ///< High priority for audio

    // 6. Start the operating system!
    qor_start(&IdleTcb, IdleTask, IdleStack, 1024);

    return 0;
}
