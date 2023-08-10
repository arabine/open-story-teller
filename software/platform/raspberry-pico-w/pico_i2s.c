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

void pico_i2s_set_frequency(const pio_i2s *i2s, const audio_i2s_config_t *config)
{
    // Pour le calcul de la fréquence, le nombre de canaux est toujours fixé à 2
    // car c'est notre format de sortie I2S
    // Dans le cas du mono, on l'a détecté en amont et on a copié l'échantillon dans la voie de droite
    float bitClk = config->freq * config->bps * config->channels /* channels */ * 2.0 /* edges per clock */;
    pio_sm_set_clkdiv(i2s->pio, i2s->sm_dout, (float)clock_get_hz(clk_sys) / bitClk);
}

//---------------------------------------------------------------------------------------------------------------------------

static void dma_double_buffer_init(pio_i2s *i2s)
{
    i2s->dma_ch_out_ctrl = dma_claim_unused_channel(true);
    i2s->dma_ch_out_data = dma_claim_unused_channel(true);

    i2s->out_ctrl_blocks[0] = i2s->output_buffer;
    i2s->out_ctrl_blocks[1] = &i2s->output_buffer[STEREO_BUFFER_SIZE];

    dma_channel_config c = dma_channel_get_default_config(i2s->dma_ch_out_ctrl);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);

    // i2s->out_ctrl_blocks pointe sur deux emplacements, 4 octets par emplacement (qui est une adresse)
    // A chaque fois que le DMA de contrôle sera activé, l'adresse va s'incrémenter de 4 octets  (DMA_SIZE_32)
    // Dès lors :
    //  - On commence à l'index 0
    //  - Puis l'index 1
    // On va revenir à l'index 0 tous les ... 8 octets, donc on programme ce décalage (3 bits)
    channel_config_set_ring(&c, false, 3);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    dma_channel_configure(i2s->dma_ch_out_ctrl, &c, &dma_hw->ch[i2s->dma_ch_out_data].al3_read_addr_trig, i2s->out_ctrl_blocks, 1, false);

    // la destination est l'alias al3_read_addr_trig qui va donc modifier l'adresse de début du transfer DMA et enclencher le démarrage
    // Nous avons deux actions, c'est pour cela que c'est un alias

    c = dma_channel_get_default_config(i2s->dma_ch_out_data);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_chain_to(&c, i2s->dma_ch_out_ctrl); // On va chaîner ce DMA au contrôle: lorsque le DMA est terminé, le DMA de contrôle va s'activer... et ainsi de suite
    channel_config_set_dreq(&c, pio_get_dreq(i2s->pio, i2s->sm_dout, true));

    dma_channel_configure(i2s->dma_ch_out_data,
                          &c,
                          &i2s->pio->txf[i2s->sm_dout],
                          NULL,
                          STEREO_BUFFER_SIZE,
                          false);
}

static void i2s_sync_program_init(PIO pio, pio_i2s *i2s, const audio_i2s_config_t *config)
{
    uint offset = 0;
    i2s->pio = pio;
    i2s->sm_mask = 0;

    i2s->sm_dout = pio_claim_unused_sm(pio, true);
    i2s->sm_mask |= (1u << i2s->sm_dout);
    offset = pio_add_program(pio0, &i2s_out_master_program);
    // 4th argument is bit depth, 5th dout, 6th bclk pin base (lrclk is bclk pin + 1)
    i2s_out_master_program_init(pio, i2s->sm_dout, offset, config->bps, config->data_pin, config->clock_pin_base);
}

void i2s_program_setup(PIO pio, void (*dma_handler)(void), pio_i2s *i2s, const audio_i2s_config_t *config)
{
    if (((uint32_t)i2s & 0x7) != 0)
    {
        debug_printf("pio_i2s argument must be 8-byte aligned!\r\n");
    }

    i2s_sync_program_init(pio, i2s, config);
    pico_i2s_set_frequency(i2s, config); // call after PIO configuration
    dma_double_buffer_init(i2s);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    pio_enable_sm_mask_in_sync(i2s->pio, i2s->sm_mask);
}

void i2s_start(pio_i2s *i2s)
{
    // D'abord on va paramétrer les diverses interruptions
    dma_channel_set_irq0_enabled(i2s->dma_ch_out_data, true);
    irq_set_enabled(DMA_IRQ_0, true);

    // YAAAAAAA on lance!
    dma_channel_start(i2s->dma_ch_out_ctrl);
}

static void pico_gracefully_stop_dma(uint channel)
{
    // See errata sheet to avoid spurious interrupts
    // disable the channel on IRQ0
    dma_channel_set_irq0_enabled(channel, false);
    // abort the channel
    dma_channel_abort(channel);
    // clear the spurious IRQ (if there was one)
    dma_channel_acknowledge_irq0(channel);
}

void i2s_stop(pio_i2s *i2s)
{
    pico_gracefully_stop_dma(i2s->dma_ch_out_ctrl);
    pico_gracefully_stop_dma(i2s->dma_ch_out_data);
    irq_set_enabled(DMA_IRQ_0, false);
}
