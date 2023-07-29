
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

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
#define VM_EV_NO_EVENT 0
#define VM_EV_START_STORY_EVENT 0xA1

typedef struct
{
    uint8_t ev;
    const char *story_dir;
} ost_vm_event_t;

// ===========================================================================================================
// PRIVATE GLOBAL VARIABLES
// ===========================================================================================================
static qor_tcb_t VmTcb;
static uint32_t VmStack[4096];
static qor_mbox_t VmMailBox;
static ost_vm_event_t VmQueue[10];

static ost_vm_event_t VmStartEvent;
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
        }
        else
        {
            //  m_ostHmiDock->ClearImage();
        }

        if (m_chip32_ctx.registers[R1] != 0)
        {
            // sound file name address is in R1
            // QString soundFile = m_model.BuildFullSoundPath(GetFileNameFromMemory(m_chip32_ctx.registers[R1]));
            // qDebug() << ", Sound: " << soundFile;
            // m_model.PlaySoundFile(soundFile);
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
    ost_vm_event_t *e = NULL;

    while (1)
    {
        uint32_t res = qor_mbox_wait(&VmMailBox, (void **)&e, 300); // On devrait recevoir un message toutes les 3ms (durée d'envoi d'un buffer I2S)

        if (res == QOR_MBOX_OK)
        {
            if (VmStartEvent.ev == VM_EV_START_STORY_EVENT)
            {
                // Launch the execution of a story
                chip32_initialize(&m_chip32_ctx);

                do
                {
                    run_result = chip32_step(&m_chip32_ctx);
                } while (run_result != VM_OK);
            }
        }
    }
}

void vm_task_start_story(const char *story_directory)
{
    VmStartEvent.story_dir = story_directory;
    qor_mbox_notify(&VmMailBox, (void **)&VmStartEvent, QOR_MBOX_OPTION_SEND_BACK);
}

void vm_task_initialize()
{
    VmStartEvent.ev = VM_EV_START_STORY_EVENT;
    qor_mbox_init(&VmMailBox, (void **)&VmQueue, 10);
    qor_create_thread(&VmTcb, VmTask, VmStack, sizeof(VmStack) / sizeof(VmStack[0]), VM_TASK_PRIORITY, "VmTask");
}
