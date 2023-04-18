/*------------------------------------------------------/
/ Copyright (c) 2020, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ff.h"

#include "i2s.h"
#include "audio.h"

#include <gd32vf103.h>

#define SIZE_OF_SAMPLES (1024)  // samples for 2ch total
#define SAMPLE_RATE     (44100)

// Audio Double Buffer from DMA transfer
int32_t audio_buf[2][SIZE_OF_SAMPLES];
// Audio Buffer for File Read
int16_t buf_16b[SIZE_OF_SAMPLES];

int32_t  DAC_ZERO_VALUE = 1;    // Non-zero value For prevending pop-noise when PCM5102A enters/exits Zero Data Detect

volatile static int count = 0;

FIL fil;
audio_info_type audio_info;
int32_t dma_trans_number;
uint16_t idx_play = 0;
int next_is_end = 0;
int playing = 0;
int pausing = 0;
int finished = 0; // means the player has finished to play whole files in the folder (by not stop)
uint32_t data_offset = 0;

static int volume = 65; // 0 ~ 100;

union U {
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
    65536
};

static uint32_t swap16b(uint32_t in_val)
{
    u.i = in_val;
    return ((uint32_t) u.s[0] << 16) | ((uint32_t) u.s[1]);
}

// Step read for LIST chunk INFO type
// (because to read chunk data at once is too heavy to continue playing)
static void step_read_list_chunk_info_type(void)
{
    FRESULT fr;     /* FatFs return code */
    UINT br;
    char chunk_id[4];
    uint32_t size;
    char str[256];

    if (audio_info.info_offset < audio_info.info_size) {
        f_lseek(&fil, audio_info.info_start + audio_info.info_offset);
        fr = f_read(&fil, chunk_id, 4, &br);
        // if (fr) printf("ERROR A: f_read %d\n\r", (int) fr);
        fr = f_read(&fil, &size, sizeof(size), &br);
        // if (fr) printf("ERROR B: f_read %d\n\r", (int) fr);
        audio_info.info_offset += 8;
        if (size < 255) {
            memset(str, 0, 256);
            fr = f_read(&fil, str, size, &br);
            // if (fr) printf("ERROR C: f_read %d\n\r", (int) fr);
            if (memcmp(chunk_id, "iart", 4) == 0 || memcmp(chunk_id, "IART", 4) == 0) {
                //printf("Artist: %s\n\r", str);
                memcpy(audio_info.artist, str, sizeof(audio_info.artist));
            } else if (memcmp(chunk_id, "inam", 4) == 0 || memcmp(chunk_id, "INAM", 4) == 0) {
                //printf("Title: %s\n\r", str);
                memcpy(audio_info.title, str, sizeof(audio_info.title));
            } else if (memcmp(chunk_id, "iprd", 4) == 0 || memcmp(chunk_id, "IPRD", 4) == 0) {
                //printf("Album: %s\n\r", str);
                memcpy(audio_info.album, str, sizeof(audio_info.album));
            } else if (memcmp(chunk_id, "iprt", 4) == 0 || memcmp(chunk_id, "IPRT", 4) == 0) {
                //printf("Number: %s\n\r", str);
                memcpy(audio_info.number, str, sizeof(audio_info.number));
            }
        }
        audio_info.info_offset += (size + 1)/2*2; // next offset must be even number
    } else { // End of Reading LIST chunk INFO type
        audio_info.info_start = 0;
    }
}

static int list_chunk_is_info_type(void)
{
    FRESULT fr;     /* FatFs return code */
    UINT br;
    char chunk_id[4];

    fr = f_read(&fil, chunk_id, 4, &br);
    // if (fr) printf("ERROR D: f_read %d\n\r", (int) fr);
    return (memcmp(chunk_id, "info", 4) == 0 || memcmp(chunk_id, "INFO", 4) == 0);
}

static int load_next_file(TCHAR *fname_ptr)
{
    int len;
    FRESULT fr;     /* FatFs return code */
    UINT br;
    char chunk_id[4];
    uint32_t size;
    uint32_t offset;

    len = strlen(fname_ptr);
    if (strncmp(&fname_ptr[len-4], ".wav", 4) == 0 || strncmp(&fname_ptr[len-4], ".WAV", 4) == 0)
    {
        memcpy(audio_info.filename, fname_ptr, 256);
        audio_info.info_start = 0;
        fr = f_open(&fil, audio_info.filename, FA_READ);
        if (fr != FR_OK) {
            // printf("ERROR: f_open %d\n\r", (int) fr);
        }
        idx_play++;
        offset = 0xc;
        f_lseek(&fil, offset);
        // Find 'data' chunk
        while (1) {
            f_read(&fil, chunk_id, 4, &br);
            f_read(&fil, &size, sizeof(size), &br);
            offset += 8;
            if (memcmp(chunk_id, "data", 4) == 0 || memcmp(chunk_id, "DATA", 4) == 0) break;
            if (memcmp(chunk_id, "list", 4) == 0 || memcmp(chunk_id, "LIST", 4) == 0) {
                if (list_chunk_is_info_type()) {
                    audio_info.info_start = offset;
                    audio_info.info_size = size;
                    audio_info.info_offset = 4; // 'LIST' -> 'INFO'
                } else {
                    audio_info.info_start = 0;
                }
            }
            offset += size;
            f_lseek(&fil, offset);
        }
        audio_info.data_size = size;
        // printf("Audio data size = %d\n\r", (int) audio_info.data_size);
        audio_info.data_start = offset;
        audio_info.data_offset = data_offset;
        if (data_offset > 0) {
            f_lseek(&fil, offset + data_offset);
        }
        data_offset = 0; // data_offset applied first file only

        return 1;
    }

    return 0;
}

static int get_level(uint32_t val)
{
    int i;
    for (i = 0; i < 101; i++) {
        if (val*2 < vol_table[i]) break;
    }
    return i;
}

// trans_number: DMA transfer count of 16bit->32bit transfer (NOT Byte count)
// but it equals Byte count of 16bit RAW data (actually equals (Byte count of 16bit RAW data)*2/2)
// because 16bit RAW data is expanded into 32bit data for 24bit DAC
static int get_audio_buf(FIL *tec, int32_t *buf_32b, int32_t *trans_number)
{
    int i;
    FRESULT fr;     /* FatFs return code */
    UINT br;
    int _next_is_end = 0; /* 0: continue, 1: next is end */
    uint32_t number;
    uint32_t file_rest;
    uint32_t trans_rest;
    uint32_t trans;
    uint32_t lvl_l = 0;
    uint32_t lvl_r = 0;

    number = 0; // number to transfer
    while (number < sizeof(buf_16b)) {
        file_rest = audio_info.data_size - audio_info.data_offset;
        trans_rest = sizeof(buf_16b) - number;
        trans = (file_rest >= trans_rest) ? trans_rest : file_rest;
        //LEDR(1);
        fr = f_read(&fil, &buf_16b[number/2], trans, &br);
        //LEDR(0);
        if (fr == FR_OK) {
            number += trans;
            audio_info.data_offset += trans;
        } else {
            // printf("ERROR: f_read %d, data_offset = %d\n\r", (int) fr, (int) audio_info.data_offset);
            f_close(&fil);
            *trans_number = number;
            return 1;
        }
        if (audio_info.data_size <= audio_info.data_offset) {
            f_close(&fil);
            _next_is_end = 1;
            break;
        }
    }

   for (i = 0; i < number/4; i++) {
        buf_32b[i*2+0] = (int32_t) swap16b((int32_t) buf_16b[i*2+0] * vol_table[volume]) + DAC_ZERO_VALUE; // L
        buf_32b[i*2+1] = (int32_t) swap16b((int32_t) buf_16b[i*2+1] * vol_table[volume]) + DAC_ZERO_VALUE; // R
        lvl_l += ((int32_t) buf_16b[i*2+0] * buf_16b[i*2+0]) / 32768;
        lvl_r += ((int32_t) buf_16b[i*2+1] * buf_16b[i*2+1]) / 32768;
    }
    audio_info.lvl_l = get_level(lvl_l/(number/4));
    audio_info.lvl_r = get_level(lvl_r/(number/4));
    *trans_number = number;
    return _next_is_end;
}

void audio_set_data_offset(uint32_t data_ofs)
{
    data_offset = data_ofs;
}

void audio_init(void)
{
    memset(audio_info.filename, 0, sizeof(audio_info.filename));
    memset(audio_info.artist, 0, sizeof(audio_info.artist));
    memset(audio_info.title, 0, sizeof(audio_info.title));
    memset(audio_info.album, 0, sizeof(audio_info.album));
    memset(audio_info.number, 0, sizeof(audio_info.number));
    count = 0;
    playing = 0;
    pausing = 0;

    for (int i = 0; i < SIZE_OF_SAMPLES; i++) {
        audio_buf[0][i] = DAC_ZERO_VALUE;
        audio_buf[1][i] = DAC_ZERO_VALUE;
    }
    dma_trans_number = SIZE_OF_SAMPLES*2;

    init_i2s2();
    spi_dma_enable(SPI2, SPI_DMA_TRANSMIT);
    init_dma_i2s2(audio_buf[0], dma_trans_number);
    dma_channel_enable(DMA1, DMA_CH1);
    dma_interrupt_enable(DMA1, DMA_CH1, DMA_INT_FTF);
    eclic_irq_enable(DMA1_Channel1_IRQn, 15, 15); // level = 15, priority = 15 (MAX)

}

int audio_play(TCHAR *fname_ptr)
{
    if (playing) return 0;

    memset(audio_info.filename, 0, sizeof(audio_info.filename));
    if (!load_next_file(fname_ptr)) {
        finished = 1;
        return 0;
    }
    memset(audio_info.artist, 0, sizeof(audio_info.artist));
    memset(audio_info.title, 0, sizeof(audio_info.title));
    memset(audio_info.album, 0, sizeof(audio_info.album));
    memset(audio_info.number, 0, sizeof(audio_info.number));

    count = 0;
    playing = 1; // After playing is set to 1, only IRQ routine can access file otherwise conflict occurs
    pausing = 0;
    next_is_end = 0;
    finished = 0;

    return 1;
}

void audio_pause(void)
{
    if (!playing) return;
    pausing = 1 - pausing;
}

void audio_stop(void)
{
    if (playing || pausing) {
        playing = 0;
        pausing = 0;
        f_close(&fil);
    }
}

void DMA1_Channel1_IRQHandler(void)
{
    int nxt1 = (count & 0x1) ^ 0x1;
    int nxt2 = 1 - nxt1;
    dma_flag_clear(DMA1, DMA_CH1, DMA_FLAG_FTF);
    dma_channel_disable(DMA1, DMA_CH1);
    if (next_is_end) {
        playing = 0;
        pausing = 0;
    }
    init_dma_i2s2(audio_buf[nxt1], dma_trans_number);
    dma_channel_enable(DMA1, DMA_CH1);
    if (playing && !pausing) {
        next_is_end = get_audio_buf(&fil, audio_buf[nxt2], &dma_trans_number);
    } else {
        for (int i = 0; i < SIZE_OF_SAMPLES; i++) {
            audio_buf[nxt2][i] = DAC_ZERO_VALUE;
        }
        dma_trans_number = SIZE_OF_SAMPLES*2;
    }
    count++;
    //dma_interrupt_flag_clear(DMA1, DMA_CH1, DMA_INT_FLAG_G);  /* not needed */
}

int audio_is_playing_or_pausing(void)
{
    return (playing || pausing);
}

int audio_is_pausing(void)
{
    return pausing;
}

int audio_finished(void)
{
    return finished;
}

uint16_t audio_get_idx_play(void)
{
    return idx_play;
}

const audio_info_type *audio_get_info(void)
{
    return (const audio_info_type *) &audio_info;
}

void volume_up(void)
{
    if (volume < 100) volume++;
}

void volume_down(void)
{
    if (volume > 0) volume--;
}

void volume_set(int val)
{
    volume = val;
}

int volume_get(void)
{
    return volume;
}
