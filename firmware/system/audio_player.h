#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <ff.h>

#define AUDIO_BUFFER_FRAMES (128)                  // in bytes
#define STEREO_BUFFER_SIZE AUDIO_BUFFER_FRAMES * 2 // for left + right
#define FILENAME_MAX_SIZE 260

typedef struct
{
    char filename[FILENAME_MAX_SIZE];
    uint32_t info_start;
    uint32_t info_size;
    uint32_t info_offset;
    uint32_t data_start;
    uint32_t data_size;
    uint32_t data_offset;
    uint32_t sample_rate;
    uint16_t channels;
    char artist[256];
    char title[256];
    char album[256];
    char number[8];
    uint32_t lvl_l;
    uint32_t lvl_r;
} audio_info_type_t;

typedef struct
{
    FIL fil;
    audio_info_type_t audio_info;
    int32_t transfer_size;
    uint16_t idx_play;
    int next_is_end;
    int playing;
    int pausing;
    int finished; // means the player has finished to play whole files in the folder (by not stop)
    uint32_t data_offset;
    uint8_t header[36];

    int count;
    int volume; // 0 ~ 100;

} audio_ctx_t;

void audio_init(audio_ctx_t *ctx);
int audio_process(audio_ctx_t *ctx);
int audio_play(audio_ctx_t *ctx, const char *fname_ptr);
void audio_pause(audio_ctx_t *ctx);
void audio_stop(audio_ctx_t *ctx);
