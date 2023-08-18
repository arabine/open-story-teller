/**
 * @file fs_task.c
 *
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-07-29
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "ost_hal.h"
#include "debug.h"
#include "qor.h"
#include "audio_player.h"
#include "filesystem.h"
#include "system.h"
#include "vm_task.h"
#include "fs_task.h"
#include "sdcard.h"

// ===========================================================================================================
// DEFINITIONS
// ===========================================================================================================
typedef enum
{
    FS_NO_EVENT,
    FS_PLAY_SOUND,
    FS_DISPLAY_IMAGE,
    FS_LOAD_INDEX,
    FS_LOAD_STORY,
    FS_READ_SDCARD_BLOCK,
    FS_WRITE_SDCARD_BLOCK,
    FS_AUDIO_NEXT_SAMPLES
} fs_state_t;

typedef struct
{
    fs_state_t ev;
    uint8_t *mem;
    uint32_t addr;
    ost_button_t button;
    char *image;
    char *sound;
    fs_result_cb_t cb;
} ost_fs_event_t;

#define ASSETS_DIR "/assets/"
#define STORY_DIR_OFFSET (UUID_SIZE + 1) // +1 for the first '/' (filesystem root)

// ===========================================================================================================
// PRIVATE GLOBAL VARIABLES
// ===========================================================================================================
static qor_tcb_t FsTcb;
static uint32_t FsStack[4096];

static qor_mbox_t FsMailBox;

static ost_fs_event_t *FsEventQueue[10];

static ost_context_t OstContext;

static int PacketCounter = 0;

static char ScratchFile[260];

static uint8_t LedState = 0;

// ===========================================================================================================
// FILE SYSTEM TASK
// ===========================================================================================================

// End of DMA transfer callback
static void audio_callback(void)
{
    static ost_fs_event_t NextSamplesEv = {
        .ev = FS_AUDIO_NEXT_SAMPLES};
    qor_mbox_notify(&FsMailBox, (void **)&NextSamplesEv, QOR_MBOX_OPTION_SEND_BACK);
}

static void show_duration(uint32_t millisecondes)
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

static bool UsbConnected = false;

void fs_task_usb_connected()
{
    UsbConnected = true;
}

void fs_task_usb_disconnected()
{
    UsbConnected = true;
}

void FsTask(void *args)
{
    ost_fs_event_t *message = NULL;
    uint32_t res = 0;

    filesystem_read_index_file(&OstContext);

    int isPlaying = 0;

    while (1)
    {
        res = qor_mbox_wait(&FsMailBox, (void **)&message, 1000);
        if (res == QOR_MBOX_OK)
        {
            switch (message->ev)
            {
            case FS_PLAY_SOUND:
                if (OstContext.sound != NULL)
                {
                    ScratchFile[STORY_DIR_OFFSET] = 0;
                    strcat(ScratchFile, ASSETS_DIR);
                    strcat(ScratchFile, message->sound);

                    debug_printf("\r\n-------------------------------------------------------\r\nPlaying: %s\r\n", ScratchFile);

                    ost_system_stopwatch_start();
                    ost_audio_play(ScratchFile);
                    PacketCounter = 0;
                    isPlaying = 1;
                }
                break;

            case FS_AUDIO_NEXT_SAMPLES:
                isPlaying = ost_audio_process();
                PacketCounter++;

                if (isPlaying == 0)
                {
                    uint32_t executionTime = ost_system_stopwatch_stop();
                    ost_audio_stop();

                    debug_printf("\r\nPackets: %d\r\n", PacketCounter);
                    show_duration(executionTime);
                    vm_task_sound_finished();
                }
                break;

            case FS_DISPLAY_IMAGE:

                if (OstContext.image != NULL)
                {
                    ScratchFile[STORY_DIR_OFFSET] = 0;
                    strcat(ScratchFile, ASSETS_DIR);
                    strcat(ScratchFile, message->image);

                    filesystem_display_image(ScratchFile);
                }
                break;

            case FS_LOAD_INDEX:
            {
                bool success = false;
                if (OstContext.number_of_stories > 0)
                {
                    filesystem_get_story_title(&OstContext);

                    // Init current directory
                    ScratchFile[0] = '/';
                    memcpy(&ScratchFile[1], OstContext.uuid, UUID_SIZE);
                    ScratchFile[STORY_DIR_OFFSET] = 0;
                    success = true;
                }

                if (message->cb != NULL)
                {
                    message->cb(success);
                }
            }
            break;
            case FS_LOAD_STORY:
                ScratchFile[STORY_DIR_OFFSET] = 0;
                strcat(ScratchFile, "/story.c32");
                filesystem_load_rom(message->mem, ScratchFile);
                // ROM loaded, execute story
                vm_task_start_story();
                break;

            case FS_READ_SDCARD_BLOCK:
                sdcard_sector_read(message->addr, message->mem);
                if (message->cb != NULL)
                {
                    message->cb(true);
                }
                break;

            case FS_WRITE_SDCARD_BLOCK:
                sdcard_sector_write(message->addr, message->mem);
                if (message->cb != NULL)
                {
                    message->cb(true);
                }
                break;

            default:
                break;
            }
        }
        else
        {
            LedState = 1 - LedState;
            ost_hal_gpio_set(OST_GPIO_DEBUG_LED, LedState);
        }
    }
}

void fs_task_read_block(uint32_t addr, uint8_t *block, fs_result_cb_t cb)
{
    static ost_fs_event_t ReadBlockEv = {
        .ev = FS_READ_SDCARD_BLOCK};

    ReadBlockEv.mem = block;
    ReadBlockEv.addr = addr;
    ReadBlockEv.cb = cb;
    qor_mbox_notify(&FsMailBox, (void **)&ReadBlockEv, QOR_MBOX_OPTION_SEND_BACK);
}

void fs_task_write_block(uint32_t addr, uint8_t *block, fs_result_cb_t cb)
{
    static ost_fs_event_t WriteBlockEv = {
        .ev = FS_WRITE_SDCARD_BLOCK};

    WriteBlockEv.mem = block;
    WriteBlockEv.addr = addr;
    WriteBlockEv.cb = cb;
    qor_mbox_notify(&FsMailBox, (void **)&WriteBlockEv, QOR_MBOX_OPTION_SEND_BACK);
}

void fs_task_scan_index(fs_result_cb_t cb)
{
    static ost_fs_event_t ScanIndexEv = {
        .ev = FS_LOAD_INDEX,
        .cb = NULL};
    ScanIndexEv.cb = cb;
    qor_mbox_notify(&FsMailBox, (void **)&ScanIndexEv, QOR_MBOX_OPTION_SEND_BACK);
}

void fs_task_play_index()
{
    fs_task_image_start(OstContext.image);
    fs_task_sound_start(OstContext.sound);
}

void fs_task_load_story(uint8_t *mem)
{
    static ost_fs_event_t LoadRomxEv = {
        .ev = FS_LOAD_STORY,
        .cb = NULL};

    LoadRomxEv.mem = mem;
    qor_mbox_notify(&FsMailBox, (void **)&LoadRomxEv, QOR_MBOX_OPTION_SEND_BACK);
}

void fs_task_sound_start(char *sound)
{
    static ost_fs_event_t MediaStartEv = {
        .ev = FS_PLAY_SOUND,
        .cb = NULL};

    MediaStartEv.image = NULL;
    MediaStartEv.sound = sound;
    qor_mbox_notify(&FsMailBox, (void **)&MediaStartEv, QOR_MBOX_OPTION_SEND_BACK);
}

void fs_task_image_start(char *image)
{
    static ost_fs_event_t MediaStartEv = {
        .ev = FS_DISPLAY_IMAGE,
        .cb = NULL};

    MediaStartEv.image = image;
    MediaStartEv.sound = NULL;
    qor_mbox_notify(&FsMailBox, (void **)&MediaStartEv, QOR_MBOX_OPTION_SEND_BACK);
}

void fs_task_initialize()
{
    qor_mbox_init(&FsMailBox, (void **)&FsEventQueue, 10);

    ost_audio_register_callback(audio_callback);

    qor_create_thread(&FsTcb, FsTask, FsStack, sizeof(FsStack) / sizeof(FsStack[0]), FS_TASK_PRIORITY, "FsTask");
}
