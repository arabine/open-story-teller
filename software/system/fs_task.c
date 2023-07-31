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

// ===========================================================================================================
// DEFINITIONS
// ===========================================================================================================
typedef struct
{
    uint8_t ev;
} ost_audio_event_t;

typedef enum
{
    FS_WAIT_FOR_EVENT,
    FS_PLAY_SOUND,
    FS_DISPLAY_IMAGE,
    FS_LOAD_INDEX,
    FS_LOAD_STORY
} fs_state_t;

typedef struct
{
    fs_state_t ev;
    uint8_t *mem;
    char *filename;
} ost_fs_event_t;

// ===========================================================================================================
// PRIVATE GLOBAL VARIABLES
// ===========================================================================================================
static qor_tcb_t FsTcb;
static uint32_t FsStack[4096];

static qor_mbox_t AudioMailBox;

static ost_audio_event_t wake_up;

static ost_audio_event_t AudioQueue[10];

static int dbg_state = 0;

static fs_state_t FsState = FS_WAIT_FOR_EVENT;

static qor_mbox_t FsMailBox;

static ost_fs_event_t FsEventQueue[10];

static ost_context_t OstContext;

// ===========================================================================================================
// FILE SYSTEM TASK
// ===========================================================================================================

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

void play_sound_file(const char *filename)
{
    debug_printf("\r\n-------------------------------------------------------\r\nPlaying: out2.wav\r\n");
    ost_system_stopwatch_start();
    ost_audio_play(filename);

    ost_audio_event_t *e = NULL;

    int isPlaying = 0;
    int count = 0;
    uint32_t res = 0;
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
}

static char ScratchFile[260];

static const char *ImagesDir = "/images/";
static const char *SoundsDir = "/sounds/";

#define STORY_DIR_OFFSET (UUID_SIZE + 1)

static uint8_t LedState = 0;

void FsTask(void *args)
{
    ost_fs_event_t *fs_ev = NULL;
    uint32_t res = 0;

    filesystem_read_index_file(&OstContext);
    FsState = FS_LOAD_INDEX;

    while (1)
    {
        switch (FsState)
        {
        case FS_PLAY_SOUND:
            ScratchFile[STORY_DIR_OFFSET] = 0;
            strcat(ScratchFile, SoundsDir);
            strcat(ScratchFile, OstContext.sound);
            play_sound_file(ScratchFile);
            FsState = FS_WAIT_FOR_EVENT;
            break;
        case FS_DISPLAY_IMAGE:
            ScratchFile[STORY_DIR_OFFSET] = 0;
            strcat(ScratchFile, ImagesDir);
            strcat(ScratchFile, OstContext.image);

            filesystem_display_image(ScratchFile);
            if (OstContext.sound != NULL)
            {
                FsState = FS_PLAY_SOUND;
            }
            else
            {
                FsState = FS_WAIT_FOR_EVENT;
            }
            break;

        case FS_LOAD_INDEX:
            if (OstContext.number_of_stories > 0)
            {
                filesystem_get_story_title(&OstContext);

                // Init current directory
                ScratchFile[0] = '/';
                memcpy(&ScratchFile[1], OstContext.uuid, UUID_SIZE);
                ScratchFile[1 + UUID_SIZE] = 0;

                // first, display image, then sound
                if (OstContext.image != NULL)
                {
                    FsState = FS_DISPLAY_IMAGE;
                }
                else if (OstContext.sound != NULL)
                {
                    FsState = FS_PLAY_SOUND;
                }
                else
                {
                    FsState = FS_WAIT_FOR_EVENT;
                }
            }
            else
            {
                FsState = FS_WAIT_FOR_EVENT;
            }
            break;
        case FS_LOAD_STORY:
            filesystem_load_rom(fs_ev->mem, fs_ev->filename);
            // ROM loaded, execute story
            FsState = FS_WAIT_FOR_EVENT;
            break;

        case FS_WAIT_FOR_EVENT:
        default:
            res = qor_mbox_wait(&FsMailBox, (void **)&fs_ev, 1000);
            if (res == QOR_MBOX_OK)
            {
                // valid event, accept it
                FsState = fs_ev->ev;
            }
            else
            {
                LedState = 1 - LedState;
                ost_hal_gpio_set(OST_GPIO_DEBUG_LED, LedState);
            }
            break;
        }
    }
}

void fs_task_scan_index()
{
    static ost_fs_event_t ScanIndexEv = {
        .ev = FS_LOAD_INDEX,
        .filename = NULL};

    qor_mbox_notify(&FsMailBox, (void **)&ScanIndexEv, QOR_MBOX_OPTION_SEND_BACK);
}

void fs_task_initialize()
{
    qor_mbox_init(&AudioMailBox, (void **)&AudioQueue, 10);
    qor_mbox_init(&FsMailBox, (void **)&FsEventQueue, 10);

    ost_audio_register_callback(audio_callback);

    qor_create_thread(&FsTcb, FsTask, FsStack, sizeof(FsStack) / sizeof(FsStack[0]), FS_TASK_PRIORITY, "FsTask");
}
