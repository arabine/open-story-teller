/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PICO_I2S_H
#define PICO_I2S_H

#include <stdbool.h>

#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pio.h"

#include "audio_player.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        uint32_t freq;
        uint32_t bps;
        uint16_t channels;
        uint8_t data_pin;
        uint8_t clock_pin_base;

    } audio_i2s_config_t;

    typedef struct pio_i2s
    {
        int32_t output_buffer[STEREO_BUFFER_SIZE * 2]; // double buffering, so x2
        int32_t *out_ctrl_blocks[2];

        PIO pio;
        uint8_t sm_mask;
        uint8_t sm_dout;
        uint dma_ch_out_ctrl;
        uint dma_ch_out_data;
        int buffer_index;

    } pio_i2s;

    void i2s_program_setup(PIO pio, void (*dma_handler)(void), pio_i2s *i2s, const audio_i2s_config_t *config);

    void pico_i2s_set_frequency(const pio_i2s *i2s, const audio_i2s_config_t *config);
    void i2s_start(pio_i2s *i2s);
    void i2s_stop(pio_i2s *i2s);

#ifdef __cplusplus
}
#endif

#endif //_AUDIO_I2S_H
