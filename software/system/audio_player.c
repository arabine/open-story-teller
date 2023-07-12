// Copyright (c) 2023, Anthony Rabine
// Copyright (c) 2020, Elehobica (original version)
// Released under the BSD-2-Clause
// refer to https://opensource.org/licenses/BSD-2-Clause

#include "audio_player.h"
#include "debug.h"
#include <string.h>

#include "ost_hal.h"
#include "libutil.h"

// 1 sample = one 16-bit data for left, one 16-bit data for right (stereo example)
#define SIZE_OF_SAMPLES (1024)

// Audio Double Buffer for DMA transfer
int32_t audio_buf[2][SIZE_OF_SAMPLES];

// int16_t audio_buf16[2][SIZE_OF_SAMPLES];
// Audio Buffer for File Read
uint8_t raw_buf[SIZE_OF_SAMPLES * 2 * 2]; // x2 for 16-bit, L+R

int32_t DAC_ZERO_VALUE = 1; //

union U
{
    uint32_t i;
    uint16_t s[2];
} u;

static const uint32_t vol_table[101] = {
    0, 4, 8, 12, 16, 20, 24, 27, 29, 31,
    34, 37, 40, 44, 48, 52, 57, 61, 67, 73,
    79, 86, 94, 102, 111, 120, 131, 142, 155, 168,
    183, 199, 217, 236, 256, 279, 303, 330, 359, 390, // vol_table[34] = 256;
    424, 462, 502, 546, 594, 646, 703, 764, 831, 904,
    983, 1069, 1163, 1265, 1376, 1496, 1627, 1770, 1925, 2094,
    2277, 2476, 2693, 2929, 3186, 3465, 3769, 4099, 4458, 4849,
    5274, 5736, 6239, 6785, 7380, 8026, 8730, 9495, 10327, 11232,
    12216, 13286, 14450, 15716, 17093, 18591, 20220, 21992, 23919, 26015,
    28294, 30773, 33470, 36403, 39592, 43061, 46835, 50938, 55402, 60256,
    65536};

static uint32_t swap16b(uint32_t in_val)
{
    u.i = in_val;
    return ((uint32_t)u.s[0] << 16) | ((uint32_t)u.s[1]);
}

void audio_init(audio_ctx_t *ctx)
{
    memset(ctx->audio_info.filename, 0, sizeof(ctx->audio_info.filename));
    memset(ctx->audio_info.artist, 0, sizeof(ctx->audio_info.artist));
    memset(ctx->audio_info.title, 0, sizeof(ctx->audio_info.title));
    memset(ctx->audio_info.album, 0, sizeof(ctx->audio_info.album));
    memset(ctx->audio_info.number, 0, sizeof(ctx->audio_info.number));

    ctx->idx_play = 0;
    ctx->next_is_end = 0;
    ctx->data_offset = 0;
    ctx->playing = 0;
    ctx->pausing = 0;
    ctx->volume = 65;
    ctx->count = 0;

    for (int i = 0; i < SIZE_OF_SAMPLES; i++)
    {
        audio_buf[0][i] = DAC_ZERO_VALUE;
        audio_buf[1][i] = DAC_ZERO_VALUE;

        // audio_buf16[0][i] = DAC_ZERO_VALUE;
        // audio_buf16[1][i] = DAC_ZERO_VALUE;
    }

    ctx->dma_trans_number = SIZE_OF_SAMPLES / 4;
}

static int list_chunk_is_info_type(audio_ctx_t *ctx)
{
    FRESULT fr; /* FatFs return code */
    UINT br;
    char chunk_id[4];

    fr = f_read(&ctx->fil, chunk_id, 4, &br);
    // if (fr) printf("ERROR D: f_read %d\n\r", (int) fr);
    return (memcmp(chunk_id, "info", 4) == 0 || memcmp(chunk_id, "INFO", 4) == 0);
}

static int load_next_file(audio_ctx_t *ctx, const char *fname_ptr)
{
    int len;
    FRESULT fr; /* FatFs return code */
    UINT br;
    char chunk_id[4];
    uint32_t size;
    uint32_t offset;

    len = strlen(fname_ptr);
    if (strncmp(&fname_ptr[len - 4], ".wav", 4) == 0 || strncmp(&fname_ptr[len - 4], ".WAV", 4) == 0)
    {
        memcpy(ctx->audio_info.filename, fname_ptr, 256);
        ctx->audio_info.info_start = 0;
        fr = f_open(&ctx->fil, ctx->audio_info.filename, FA_READ);
        if (fr != FR_OK)
        {
            debug_printf("ERROR: f_open %d\n\r", (int)fr);
        }
        ctx->idx_play++;
        FRESULT res = f_read(&ctx->fil, ctx->header, sizeof(ctx->header), &br);
        // Find 'data' chunk

        offset = 0;
        while (1)
        {
            res = f_read(&ctx->fil, chunk_id, 4, &br);
            res = f_read(&ctx->fil, &size, 4, &br);
            offset += 8;
            if (res == FR_OK)
            {
                if (memcmp(chunk_id, "data", 4) == 0 || memcmp(chunk_id, "DATA", 4) == 0)
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        ctx->audio_info.data_size = size;
        // printf("Audio data size = %d\n\r", (int) audio_info.data_size);
        ctx->audio_info.data_start = offset;
        ctx->audio_info.data_offset = 0;
        return 1;
    }

    return 0;
}

static int get_level(uint32_t val)
{
    int i;
    for (i = 0; i < 101; i++)
    {
        if (val * 2 < vol_table[i])
            break;
    }
    return i;
}

int32_t swp(int16_t data)
{
    int32_t r = (data & 0xFF) << 8;
    r = r | ((data >> 8) & 0xFF);
    return r;
}

// trans_number: DMA transfer count of 16bit->32bit transfer (NOT Byte count)
// but it equals Byte count of 16bit RAW data (actually equals (Byte count of 16bit RAW data)*2/2)
// because 16bit RAW data is expanded into 32bit data for 24bit DAC
static int get_audio_buf(audio_ctx_t *ctx, int32_t *buf_32b)
{
    int i;
    FRESULT fr; /* FatFs return code */
    UINT br;
    int _next_is_end = 0; /* 0: continue, 1: next is end */
    uint32_t number;
    uint32_t file_rest;
    uint32_t trans_rest;
    uint32_t trans;
    uint32_t lvl_l = 0;
    uint32_t lvl_r = 0;

    number = 0; // number to transfer (en octets)
    while (number < sizeof(raw_buf))
    {
        file_rest = ctx->audio_info.data_size - ctx->audio_info.data_offset;
        trans_rest = sizeof(raw_buf) - number; // en octets, 2048 au début
        trans = (file_rest >= trans_rest) ? trans_rest : file_rest;
        // LEDR(1);
        fr = f_read(&ctx->fil, &raw_buf[number], trans, &br);
        // LEDR(0);
        if (fr == FR_OK)
        {
            number += trans;
            ctx->audio_info.data_offset += trans;
        }
        else
        {
            debug_printf("ERROR: f_read %d, data_offset = %d\n\r", (int)fr, (int)ctx->audio_info.data_offset);
            f_close(&ctx->fil);
            ctx->dma_trans_number = number / 4;
            return 1;
        }
        if (ctx->audio_info.data_size <= ctx->audio_info.data_offset)
        {
            f_close(&ctx->fil);
            _next_is_end = 1;
            break;
        }
    }

    static bool onetime = true;

    // samples : total bytes devided by 2 (16 bits) and by two again (2 channels)
    for (i = 0; i < number / 4; i++)
    {
        // buf_32b[i * 2 + 0] = (int32_t)swap16b((int32_t)buf_16b[i * 2 + 0] * vol_table[ctx->volume]) + DAC_ZERO_VALUE; // L
        // buf_32b[i * 2 + 1] = (int32_t)swap16b((int32_t)buf_16b[i * 2 + 1] * vol_table[ctx->volume]) + DAC_ZERO_VALUE; // R

        // Avec le AUDIO PICO de waveshare, on entend un truc

        buf_32b[i * 2] = ((int32_t)((int16_t)leu16_get(&raw_buf[i * 4])));
        buf_32b[i * 2 + 1] = ((int32_t)((int16_t)leu16_get(&raw_buf[i * 4 + 2])));

        // buf_32b[i * 2] = 1;
        // buf_32b[i * 2 + 1] = 4;

        // lvl_l += ((int32_t)buf_16b[i * 2 + 0] * buf_16b[i * 2 + 0]) / 32768;
        // lvl_r += ((int32_t)buf_16b[i * 2 + 1] * buf_16b[i * 2 + 1]) / 32768;
    }
    /*
        if (onetime)
        {
            onetime = false;

            for (int i = 0; i < 10; i++)
            {
                debug_printf("Sample left:  %d\r\n", buf_32b[i * 2]);
                debug_printf("Sample right: %d\r\n", buf_32b[i * 2 + 1]);
            }
        }
    */
    // ctx->audio_info.lvl_l = get_level(lvl_l / (number / 4));
    // ctx->audio_info.lvl_r = get_level(lvl_r / (number / 4));
    ctx->dma_trans_number = (number / 4); // 32 bytes tranfers
    return _next_is_end;
}

void audio_process(audio_ctx_t *ctx)
{
    int nxt1 = (ctx->count & 0x1) ^ 0x1;
    int nxt2 = 1 - nxt1;
    // dma_flag_clear(DMA1, DMA_CH1, DMA_FLAG_FTF);
    // dma_channel_disable(DMA1, DMA_CH1);

     ost_hal_audio_frame_end();

    if (ctx->next_is_end)
    {
        ctx->playing = 0;
        ctx->pausing = 0;
    }
    // init_dma_i2s2(audio_buf[nxt1], dma_trans_number);
    // dma_channel_enable(DMA1, DMA_CH1);

    ost_hal_audio_frame_start(audio_buf[nxt1], ctx->dma_trans_number);

    if (ctx->playing && !ctx->pausing)
    {
        ctx->next_is_end = get_audio_buf(ctx, audio_buf[nxt2]);
    }
    else
    {
        for (int i = 0; i < SIZE_OF_SAMPLES; i++)
        {
            audio_buf[nxt2][i] = DAC_ZERO_VALUE;
        }
        ctx->dma_trans_number = SIZE_OF_SAMPLES / 4;
    }
    ctx->count++;
    // dma_interrupt_flag_clear(DMA1, DMA_CH1, DMA_INT_FLAG_G);  /* not needed */
}

int audio_play(audio_ctx_t *ctx, const char *fname_ptr)
{
    if (ctx->playing)
        return 0;

    memset(ctx->audio_info.filename, 0, sizeof(ctx->audio_info.filename));
    if (!load_next_file(ctx, fname_ptr))
    {
        ctx->finished = 1;
        return 0;
    }
    memset(ctx->audio_info.artist, 0, sizeof(ctx->audio_info.artist));
    memset(ctx->audio_info.title, 0, sizeof(ctx->audio_info.title));
    memset(ctx->audio_info.album, 0, sizeof(ctx->audio_info.album));
    memset(ctx->audio_info.number, 0, sizeof(ctx->audio_info.number));

    ctx->count = 0;
    ctx->playing = 1; // After playing is set to 1, only IRQ routine can access file otherwise conflict occurs
    ctx->pausing = 0;
    ctx->next_is_end = 0;
    ctx->finished = 0;

    return 1;
}

void audio_pause(audio_ctx_t *ctx)
{
    if (!ctx->playing)
        return;
    ctx->pausing = 1 - ctx->pausing;
}

void audio_stop(audio_ctx_t *ctx)
{
    if (ctx->playing || ctx->pausing)
    {
        ctx->playing = 0;
        ctx->pausing = 0;
        f_close(&ctx->fil);
    }
}