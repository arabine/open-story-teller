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



static char root_dir[260];
static bool gMusicLoaded = false;
char fileNameToLoad[512] = { 0 };

typedef void (*media_callback)(int32_t, const char*);

static media_callback gMediaCallback = nullptr;

//---------------------------------------------------------------------------------------
// VM Stuff
//---------------------------------------------------------------------------------------
uint8_t rom_data[16*1024];
uint8_t ram_data[16*1024];
chip32_ctx_t chip32_ctx;


chip32_result_t run_result;


static uint8_t IndexBuf[260];
static uint8_t ImageBuf[100];
static uint8_t SoundBuf[100];


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
        printf("SYSCALL 1\n");
        fflush(stdout);
//        UnloadTexture(*tex);
//        *tex =

        if (ctx->registers[R0] != 0)
        {
            // sound file name address is in R1
            char image[100];
            get_filename_from_memory(ctx, ctx->registers[R0], image);

            if (gMediaCallback)
            {
                gMediaCallback(0, image);
            }


         //   texture = LoadTexture(image_path); // FIXME
        }
        else
        {
            // UnloadTexture(texture); // FIXME
        }


        if (ctx->registers[R1] != 0)
        {
            // sound file name address is in R1
            char sound[100];
            get_filename_from_memory(ctx, ctx->registers[R1], sound);

            if (gMediaCallback)
            {
                gMediaCallback(1, sound);
            }

            // gMusic = LoadMusicStream(sound_path);
            // gMusic.looping = false;
            // gMusicLoaded = true;


// FIXME
            // if (IsMusicReady(gMusic))
            // {
            //     PlayMusicStream(gMusic);
            // }
        }
        retCode = SYSCALL_RET_WAIT_EV; // set the VM in pause
    }
    else if (code == 2) // Wait Event
    {
        printf("SYSCALL 2\n");
        fflush(stdout);
        retCode = SYSCALL_RET_WAIT_EV; // set the VM in pause
    }
    return retCode;
}


extern "C"  void storyvm_run()
{
    // VM next instruction
    if (run_result == VM_OK)
    {
        run_result = chip32_step(&chip32_ctx);
    }
}


extern "C"  void storyvm_stop()
{
    run_result = VM_FINISHED;
}


extern "C" void storyvm_initialize(media_callback cb)
{
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
     
    std::cout << "[STORYVM] Initialized" << std::endl;
}


extern "C"  void storyvm_send_event(int event)
{
}

extern "C"  void storyvm_start(const uint8_t *data, uint32_t size)
{
    if (size <= chip32_ctx.rom.size)
    {
        memcpy(chip32_ctx.rom.mem, data, size);
        run_result = VM_OK;
        chip32_initialize(&chip32_ctx);
    }
    else
    {
        run_result = VM_FINISHED;
    }

     std::cout << "[STORYVM] Start" << std::endl;
}

