/**
 * @file hmi_task.c
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
} ost_hmi_event_t;

typedef enum
{
    OST_SYS_WAIT_INDEX,
    OST_SYS_PLAY_STORY_TITLE,
    OST_SYS_WAIT_USER_EVENT
} ost_system_state_t;

// ===========================================================================================================
// GLOBAL STORY VARIABLES
// ===========================================================================================================

static qor_tcb_t HmiTcb;
static uint32_t HmiStack[4096];

static qor_mbox_t HmiMailBox;

static ost_hmi_event_t HmiEvent;

static ost_hmi_event_t *HmiQueue[10];

static ost_system_state_t OstState = OST_SYS_WAIT_INDEX;

static ost_context_t OstContext;

// ===========================================================================================================
// HMI TASK (user interface, buttons manager, LCD)
// ===========================================================================================================
void HmiTask(void *args)
{

    ost_hmi_event_t *e = NULL;

    // filesystem_display_image("/ba869e4b-03d6-4249-9202-85b4cec767a7/images/bird.qoi");

    // Start by scanning the index file
    // fs_task_scan_index();

    while (1)
    {
        uint32_t res = qor_mbox_wait(&HmiMailBox, (void **)&e, 1000);

        if (res == QOR_MBOX_OK)
        {
            switch (OstState)
            {
            case OST_SYS_PLAY_STORY_TITLE:
                break;

            case OST_SYS_WAIT_USER_EVENT:
                break;

            case OST_SYS_WAIT_INDEX:
            default:
                break;
            }
        }
        else
        {
            debug_printf("H"); // pour le debug only
        }
    }
}

void hmi_task_ost_ready(uint32_t number_of_stories)
{
    static ost_hmi_event_t OsReadyEv = {
        .ev = OST_SYS_PLAY_STORY_TITLE};

    OstContext.number_of_stories = number_of_stories;
    OstContext.current_story = 0;
    qor_mbox_notify(&HmiMailBox, (void **)&OsReadyEv, QOR_MBOX_OPTION_SEND_BACK);
}

void hmi_task_initialize()
{
    OstState = OST_SYS_WAIT_INDEX;
    qor_mbox_init(&HmiMailBox, (void **)&HmiQueue, 10);

    qor_create_thread(&HmiTcb, HmiTask, HmiStack, sizeof(HmiStack) / sizeof(HmiStack[0]), HMI_TASK_PRIORITY, "HmiTask"); // less priority is the HMI (user inputs and LCD)
}
