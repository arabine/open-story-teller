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
#include "tusb.h"

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

static qor_tcb_t UsbTcb;
static uint32_t UsbStack[4096];

static qor_mbox_t UsbMailBox;

static ost_hmi_event_t UsbEvent;

static ost_hmi_event_t *UsbQueue[10];

static ost_system_state_t OstState = OST_SYS_WAIT_INDEX;

static ost_context_t OstContext;

// ===========================================================================================================
// HMI TASK (user interface, buttons manager, LCD)
// ===========================================================================================================
void UsbTask(void *args)
{

    ost_hmi_event_t *e = NULL;

    // init device stack on configured roothub port
    tusb_init();

    while (1)
    {
        // tud_task(); // tinyusb device task
        qor_sleep(10);
    }
}

#include "msc_disk.h"

void usb_task_initialize()
{
    OstState = OST_SYS_WAIT_INDEX;
    qor_mbox_init(&UsbMailBox, (void **)&UsbQueue, 10);

    msc_disk_initialize();

    qor_create_thread(&UsbTcb, UsbTask, UsbStack, sizeof(UsbStack) / sizeof(UsbStack[0]), HMI_TASK_PRIORITY, "UsbTask"); // less priority is the HMI (user inputs and LCD)
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
    // blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    // blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void)remote_wakeup_en;
    //  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    // blink_interval_ms = BLINK_MOUNTED;
}
