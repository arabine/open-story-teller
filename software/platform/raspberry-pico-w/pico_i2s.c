/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include "pico_i2s.h"
#include "pico_i2s.pio.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"

#include "debug.h"

void pico_i2s_set_frequency(const audio_i2s_config_t *config)
{
    float bitClk = config->freq * config->bps * 2.0 /* channels */ * 2.0 /* edges per clock */;
    //  pio_sm_set_clkdiv(audio_pio, shared_state.pio_sm, (float)clock_get_hz(clk_sys) / bitClk);
}

#if 0

static bool audio_enabled;

void audio_i2s_set_enabled(bool enabled)
{
    if (enabled != audio_enabled)
    {
#ifndef NDEBUG
        if (enabled)
        {
            puts("Enabling PIO I2S audio\n");
            printf("(on core %d\n", get_core_num());
        }
#endif
        irq_set_enabled(DMA_IRQ_0 + PICO_AUDIO_I2S_DMA_IRQ, enabled);

        if (enabled)
        {
            //  audio_start_dma_transfer();
        }
        else
        {
            /*
            // if there was a buffer in flight, it will not be freed by DMA IRQ, let's do it manually
            if (shared_state.playing_buffer)
            {
                give_audio_buffer(audio_i2s_consumer, shared_state.playing_buffer);
                shared_state.playing_buffer = NULL;
            }
            */
        }

        pio_sm_set_enabled(audio_pio, shared_state.pio_sm, enabled);

        audio_enabled = enabled;
    }
}
#endif

//---------------------------------------------------------------------------------------------------------------------------

static void dma_double_buffer_init(pio_i2s *i2s, void (*dma_handler)(void))
{
    i2s->dma_ch_out_ctrl = dma_claim_unused_channel(true);
    i2s->dma_ch_out_data = dma_claim_unused_channel(true);

    i2s->out_ctrl_blocks[0] = i2s->output_buffer;
    i2s->out_ctrl_blocks[1] = &i2s->output_buffer[STEREO_BUFFER_SIZE];

    dma_channel_config c = dma_channel_get_default_config(i2s->dma_ch_out_ctrl);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_ring(&c, false, 3);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    dma_channel_configure(i2s->dma_ch_out_ctrl, &c, &dma_hw->ch[i2s->dma_ch_out_data].al3_read_addr_trig, i2s->out_ctrl_blocks, 1, false);

    c = dma_channel_get_default_config(i2s->dma_ch_out_data);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_chain_to(&c, i2s->dma_ch_out_ctrl);
    channel_config_set_dreq(&c, pio_get_dreq(i2s->pio, i2s->sm_dout, true));

    dma_channel_configure(i2s->dma_ch_out_data,
                          &c,
                          &i2s->pio->txf[i2s->sm_dout],
                          NULL,
                          STEREO_BUFFER_SIZE,
                          false);

    dma_channel_set_irq0_enabled(i2s->dma_ch_out_data, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    dma_channel_start(i2s->dma_ch_out_ctrl);
}

static void i2s_sync_program_init(PIO pio, pio_i2s *i2s, audio_i2s_config_t *config)
{
    uint offset = 0;
    i2s->pio = pio;
    i2s->sm_mask = 0;

    i2s->sm_dout = pio_claim_unused_sm(pio, true);
    i2s->sm_mask |= (1u << i2s->sm_dout);
    offset = pio_add_program(pio0, &i2s_out_master_program);
    // 4th argument is bit depth, 5th dout, 6th bclk pin base (lrclk is bclk pin + 1)
    i2s_out_master_program_init(pio, i2s->sm_dout, offset, config->bps, config->data_pin, config->clock_pin_base);
    //  pio_sm_set_clkdiv(pio, i2s->sm_dout, 89); // Approximately 11KHz audio
}

void i2s_program_start_synched(PIO pio, void (*dma_handler)(void), pio_i2s *i2s, audio_i2s_config_t *config)
{
    // i2s_sync_program_init(pio, i2s);
    pico_i2s_set_frequency(config);
    dma_double_buffer_init(i2s, dma_handler);
    pio_enable_sm_mask_in_sync(i2s->pio, i2s->sm_mask);
}
