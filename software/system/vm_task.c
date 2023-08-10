
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "ost_hal.h"
#include "debug.h"
#include "qor.h"
#include "chip32_vm.h"
#include "system.h"
#include "vm_task.h"
#include "fs_task.h"

// ===========================================================================================================
// DEFINITIONS
// ===========================================================================================================

typedef enum
{
    VM_EV_NO_EVENT = 0,
    VM_EV_START_STORY_EVENT = 0xA1,
    VM_EV_EXEC_HOME_INDEX = 0xB5,
    VM_EV_BUTTON_EVENT = 0x88,
    VM_EV_END_OF_SOUND = 0x4E,
    VM_EV_ERROR = 0xE0
} ost_vm_ev_type_t;
typedef struct
{
    uint32_t button_mask;
    ost_vm_ev_type_t ev;
    const char *story_dir;
} ost_vm_event_t;

typedef enum
{
    OST_VM_STATE_HOME,
    OST_VM_STATE_HOME_WAIT_LOAD_STORY,
    OST_VM_STATE_RUN_STORY,
    OST_VM_STATE_WAIT_BUTTON,
    OST_VM_STATE_ERROR //!< General error, cannot continue

} ost_vm_state_t;

// ===========================================================================================================
// PRIVATE GLOBAL VARIABLES
// ===========================================================================================================
static qor_tcb_t VmTcb;
static uint32_t VmStack[4096];
static qor_mbox_t VmMailBox;
static ost_vm_event_t *VmQueue[10];

static uint8_t m_rom_data[16 * 1024];
static uint8_t m_ram_data[16 * 1024];
static chip32_ctx_t m_chip32_ctx;
static char CurrentStory[260]; // Current story path
static char ImageFile[260];
static char SoundFile[260];

// ===========================================================================================================
// VIRTUAL MACHINE TASK
// ===========================================================================================================

void get_file_from_memory(char *mem, uint32_t addr)
{
    bool isRam = addr & 0x80000000;
    addr &= 0xFFFF; // mask the RAM/ROM bit, ensure 16-bit addressing
    if (isRam)
    {
        strcpy(&mem[0], (const char *)&m_chip32_ctx.ram.mem[addr]);
    }
    else
    {
        strcpy(&mem[0], (const char *)&m_chip32_ctx.rom.mem[addr]);
    }
}

// Callback from the VM
// Called inside the thread context
uint8_t vm_syscall(chip32_ctx_t *ctx, uint8_t code)
{
    uint8_t retCode = SYSCALL_RET_OK;

    // Media
    if (code == 1) // Execute media
    {
        if (m_chip32_ctx.registers[R0] != 0)
        {
            // image file name address is in R0
            // QString imageFile = m_model.BuildFullImagePath(GetFileNameFromMemory(m_chip32_ctx.registers[R0]));
            // m_ostHmiDock->SetImage(imageFile);
            get_file_from_memory(ImageFile, m_chip32_ctx.registers[R0]);
            fs_task_image_start(ImageFile);
        }

        if (m_chip32_ctx.registers[R1] != 0)
        {
            // sound file name address is in R1
            // QString soundFile = m_model.BuildFullSoundPath(GetFileNameFromMemory(m_chip32_ctx.registers[R1]));
            // qDebug() << ", Sound: " << soundFile;
            // m_model.PlaySoundFile(soundFile);
            get_file_from_memory(SoundFile, m_chip32_ctx.registers[R1]);
            fs_task_sound_start(SoundFile);
        }
        retCode = SYSCALL_RET_WAIT_EV; // set the VM in pause
    }
    // WAIT EVENT bits:
    // 0: block
    // 1: OK button
    // 2: home button
    // 3: pause button
    // 4: rotary left
    // 5: rotary right
    else if (code == 2) // Wait for event
    {
        // Event mask is located in R0
        // optional timeout is located in R1
        // if timeout is set to zero, wait for infinite and beyond
        retCode = SYSCALL_RET_WAIT_EV; // set the VM in pause
    }

    return retCode;
}

static void button_callback(uint32_t flags)
{
    static ost_vm_event_t ButtonEv = {
        .ev = VM_EV_BUTTON_EVENT,
        .story_dir = NULL};

    ButtonEv.button_mask = flags;
    qor_mbox_notify(&VmMailBox, (void **)&ButtonEv, QOR_MBOX_OPTION_SEND_BACK);
}

static void read_index_callback(bool success)
{
    static ost_vm_event_t ReadIndexEv = {
        .ev = VM_EV_BUTTON_EVENT,
        .story_dir = NULL};

    ReadIndexEv.ev = success ? VM_EV_EXEC_HOME_INDEX : VM_EV_ERROR;
    qor_mbox_notify(&VmMailBox, (void **)&ReadIndexEv, QOR_MBOX_OPTION_SEND_BACK);
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

    chip32_result_t run_result;
    ost_vm_event_t *message = NULL;
    uint32_t res = 0;

    ost_vm_state_t VmState = OST_VM_STATE_HOME;
    fs_task_scan_index(read_index_callback);
    bool run_script = false;
    bool block_keys = false;

    while (1)
    {
        res = qor_mbox_wait(&VmMailBox, (void **)&message, 300); // On devrait recevoir un message toutes les 3ms (durée d'envoi d'un buffer I2S)

        if (res == QOR_MBOX_OK)
        {
            switch (VmState)
            {
            case OST_VM_STATE_HOME:
                switch (message->ev)
                {
                case VM_EV_END_OF_SOUND:
                    block_keys = false;
                    break;

                case VM_EV_EXEC_HOME_INDEX:
                    // La lecture de l'index est terminée, on demande l'affichage des médias
                    if (!block_keys)
                    {
                        block_keys = true;
                        fs_task_play_index();
                    }
                    break;
                case VM_EV_BUTTON_EVENT:
                    if (!block_keys)
                    {
                        // debug_printf("B: %x", message->button_mask);
                        if ((message->button_mask & OST_BUTTON_OK) == OST_BUTTON_OK)
                        {
                            VmState = OST_VM_STATE_HOME_WAIT_LOAD_STORY;
                            debug_printf("OK\r\n");
                            fs_task_load_story(m_rom_data);
                        }
                    }
                    break;
                case VM_EV_ERROR:
                default:
                    break;
                }
            case OST_VM_STATE_HOME_WAIT_LOAD_STORY:
                if (message->ev == VM_EV_START_STORY_EVENT)
                {
                    // Launch the execution of a story
                    chip32_initialize(&m_chip32_ctx);

                    VmState = OST_VM_STATE_RUN_STORY;
                    run_script = true;
                }

            // Pas de break, on enchaîne sur le déroulement du script
            case OST_VM_STATE_RUN_STORY:
            {

                if (message->ev == VM_EV_END_OF_SOUND)
                {
                    run_script = true;
                }

                if (message->ev == VM_EV_BUTTON_EVENT)
                {
                    if ((message->button_mask & OST_BUTTON_OK) == OST_BUTTON_OK)
                    {
                        debug_printf("OK\r\n");
                        m_chip32_ctx.registers[R0] = 0x01;
                        run_script = true;
                    }
                    else if ((message->button_mask & OST_BUTTON_LEFT) == OST_BUTTON_LEFT)
                    {
                        debug_printf("<-\r\n");
                        m_chip32_ctx.registers[R0] = 0x02;
                        run_script = true;
                    }
                    else if ((message->button_mask & OST_BUTTON_RIGHT) == OST_BUTTON_RIGHT)
                    {
                        debug_printf("->\r\n");
                        m_chip32_ctx.registers[R0] = 0x04;
                        run_script = true;
                    }
                }

                if (run_script)
                {
                    do
                    {
                        run_result = chip32_step(&m_chip32_ctx);

                    } while (run_result == VM_OK);
                }
                run_script = false;

                break;
            }
            default:
                break;
            }
        }
    }
}

void vm_task_start_story()
{
    static ost_vm_event_t VmStartEvent;
    VmStartEvent.ev = VM_EV_START_STORY_EVENT;
    qor_mbox_notify(&VmMailBox, (void **)&VmStartEvent, QOR_MBOX_OPTION_SEND_BACK);
}

void vm_task_sound_finished()
{
    static ost_vm_event_t VmEndOfSoundEvent;
    VmEndOfSoundEvent.ev = VM_EV_END_OF_SOUND;
    qor_mbox_notify(&VmMailBox, (void **)&VmEndOfSoundEvent, QOR_MBOX_OPTION_SEND_BACK);
}

void vm_task_initialize()
{
    qor_mbox_init(&VmMailBox, (void **)&VmQueue, 10);
    qor_create_thread(&VmTcb, VmTask, VmStack, sizeof(VmStack) / sizeof(VmStack[0]), VM_TASK_PRIORITY, "VmTask");

    ost_button_register_callback(button_callback);
}
