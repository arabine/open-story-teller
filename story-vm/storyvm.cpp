// testffi/libs/api.cpp


#include <cstring>
#include <cstdio>
#include <thread>
#include <atomic>
#include <iostream>
#include <chrono>
#include <stdbool.h>

#include "chip32_vm.h"
#include "dlib_export.h"
#include "story_machine.h"


static char root_dir[260];
static bool gMusicLoaded = false;
char fileNameToLoad[512] = { 0 };

typedef void (*media_callback)(int32_t, const char*);

static media_callback gMediaCallback = nullptr;

//---------------------------------------------------------------------------------------
// VM Stuff
//---------------------------------------------------------------------------------------
static uint8_t rom_data[16*1024];
static uint8_t ram_data[16*1024];
static chip32_ctx_t chip32_ctx;


static chip32_result_t run_result;
static uint32_t event_mask = 0;

static uint8_t IndexBuf[260];
static uint8_t ImageBuf[100];
static uint8_t SoundBuf[100];

static bool IsValidEvent(uint32_t event) {
    return (event_mask & event) != 0;
}


int get_filename_from_memory(chip32_ctx_t *ctx, uint32_t addr, char *filename_mem)
{
    int valid = 0;

    // Test if address is valid

    bool isRam = addr & 0x80000000;
    addr &= 0xFFFF; // mask the RAM/ROM bit, ensure 16-bit addressing
    if (isRam) {
        strcpy(&filename_mem[0], (const char *)&ctx->ram.mem[addr]);
    } else {
        strcpy(&filename_mem[0], (const char *)&ctx->rom.mem[addr]);
    }

    return valid;
}




uint8_t story_player_syscall(chip32_ctx_t *ctx, uint8_t code)
{
    uint8_t retCode = SYSCALL_RET_OK;

    static char image_path[260];
    static char sound_path[260];

    if (code == 1) //  // Execute media
    {
        std::cout << "[STORYVM] Syscall 1" << std::endl;
        // for (int i = 0; i< REGISTER_COUNT; i++) {
        //     std::cout << "[STORYVM] Reg: " << i << ", value: " << (int)ctx->registers[i] << std::endl;
        // }

        if (ctx->registers[R0] != 0)
        {
            // sound file name address is in R1
            char image[100];
            get_filename_from_memory(ctx, ctx->registers[R0], image);

            if (gMediaCallback)
            {
                std::cout << "[STORYVM] Execute callback (image)" << std::endl;
                gMediaCallback(0, image);
            }
        }
        else
        {
            std::cout << "[STORYVM] No image" << std::endl;
        }

        if (ctx->registers[R1] != 0)
        {
            // sound file name address is in R1
            char sound[100];
            get_filename_from_memory(ctx, ctx->registers[R1], sound);

            if (gMediaCallback)
            {
                std::cout << "[STORYVM] Execute callback (sound)" << std::endl;
                gMediaCallback(1, sound);
            }
        }
        else
        {
            std::cout << "[STORYVM] No sound" << std::endl;
        }

        retCode = SYSCALL_RET_OK; // set the VM in pause
    }
    else if (code == 2) // Wait Event
    {
        std::cout << "[STORYVM] Syscall 2 (wait for event)" << std::endl;
        event_mask = ctx->registers[R0];
        retCode = SYSCALL_RET_WAIT_EV; // set the VM in pause
    }
    else if (code == 3) // Signal
    {
        if (ctx->registers[R0] == 1)
        {
            // EXIT
            std::cout << "[STORYVM] Syscall 3 (exit)" << std::endl;
            if (gMediaCallback)
            {
                std::cout << "[STORYVM] Execute callback (sound)" << std::endl;
                gMediaCallback(2, "");
            }
        }
    }
    return retCode;
}


extern "C"  void storyvm_run()
{
    // VM next instruction
    if (run_result == VM_OK)
    {
        run_result = chip32_step(&chip32_ctx);

        // for (int i = 0; i< REGISTER_COUNT; i++) {
        //     std::cout << "[STORYVM] Reg: " << i << ", value: " << (int)chip32_ctx.registers[i] << std::endl;
        // }
    }
}


extern "C"  void storyvm_stop()
{
    std::cout << "[STORYVM] Stop: " << std::endl;
    run_result = VM_FINISHED;
}


extern "C" void storyvm_initialize(media_callback cb)
{
    std::cout << "[STORYVM] Initialize: " << (void *)cb << std::endl;
    gMediaCallback = cb;

    chip32_ctx.stack_size = 512;

    chip32_ctx.rom.mem = rom_data;
    chip32_ctx.rom.addr = 0;
    chip32_ctx.rom.size = sizeof(rom_data);

    chip32_ctx.ram.mem = ram_data;
    chip32_ctx.ram.addr = sizeof(rom_data);
    chip32_ctx.ram.size = sizeof(ram_data);

    chip32_ctx.syscall = story_player_syscall;

    run_result = VM_FINISHED;

    storyvm_stop();
}

extern "C" DLIB_EXPORT void storyvm_send_event(int event)
{
    if (IsValidEvent(event))
    {
        chip32_ctx.registers[R0] = event;
        run_result = VM_OK;
    }
    else
    {
        std::cout << "[STORYVM] Invalid event" << std::endl;
    }
}

extern "C" DLIB_EXPORT void storyvm_start(const uint8_t *data, uint32_t size)
{
    if (size <= chip32_ctx.rom.size)
    {
        memcpy(chip32_ctx.rom.mem, data, size);
        run_result = VM_OK;
        chip32_initialize(&chip32_ctx);
        std::cout << "[STORYVM] Start" << std::endl;
    }
    else
    {
        run_result = VM_FINISHED;
        std::cout << "[STORYVM] Not started (not enough memory)" << std::endl;
    }
}

