/*------------------------------------------------------/
/ Copyright (c) 2020, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#ifndef _AUDIO_BUF_H_
#define _AUDIO_BUF_H_

#include <stdint.h>
#include <ff.h>

typedef struct {
    char filename[FF_LFN_BUF+1];
    uint32_t info_start;
    uint32_t info_size;
    uint32_t info_offset;
    uint32_t data_start;
    uint32_t data_size;
    uint32_t data_offset;
    char artist[256];
    char title[256];
    char album[256];
    char number[8];
    uint32_t lvl_l;
    uint32_t lvl_r;
} audio_info_type;

void audio_set_data_offset(uint32_t data_ofs);
void audio_init(void);
int audio_play(TCHAR *fname_ptr);
void audio_pause(void);
void audio_stop(void);
int audio_is_playing_or_pausing(void);
int audio_is_pausing(void);
int audio_finished(void);
uint16_t audio_get_idx_play(void);
const audio_info_type *audio_get_info(void);
void volume_up(void);
void volume_down(void);
void volume_set(int val);
int volume_get(void);

#endif
