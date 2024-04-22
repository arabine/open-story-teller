

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/dma.h"

// Grab some unused dma channels
static uint dma_tx;
static uint dma_rx;

void pico_sdcard_dma_initialize()
{
    dma_tx = dma_claim_unused_channel(true);
    dma_rx = dma_claim_unused_channel(true);

    // We set the outbound DMA to transfer from a memory buffer to the SPI transmit FIFO paced by the SPI TX FIFO DREQ
    // The default is for the read address to increment every element (in this case 1 byte = DMA_SIZE_8)
    // and for the write address to remain unchanged.

    // printf("Configure TX DMA\n");
    dma_channel_config c = dma_channel_get_default_config(dma_tx);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(spi0, true));
    dma_channel_configure(dma_tx, &c,
                          &spi_get_hw(spi0)->dr, // write address
                          NULL,                  // read address
                          0,                     // element count will be set later
                          false);                // don't start yet

    // printf("Configure RX DMA\n");

    // We set the inbound DMA to transfer from the SPI receive FIFO to a memory buffer paced by the SPI RX FIFO DREQ
    // We configure the read address to remain unchanged for each element, but the write
    // address to increment (so data is written throughout the buffer)
    c = dma_channel_get_default_config(dma_rx);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(spi0, false));
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    dma_channel_configure(dma_rx, &c,
                          NULL,                  // write address
                          &spi_get_hw(spi0)->dr, // read address
                          0,                     // element count (each element is of size transfer_data_size)
                          false);                // don't start yet

    //  printf("Starting DMAs...\n");
    // start them exactly simultaneously to avoid races (in extreme cases the FIFO could overflow)
    dma_start_channel_mask((1u << dma_tx) | (1u << dma_rx));
    //  printf("Wait for RX complete...\n");

    /*
      dma_channel_wait_for_finish_blocking(dma_rx);
      if (dma_channel_is_busy(dma_tx))
      {
          panic("RX completed before TX");
      }
      */
}

void pico_sdcard_dma_start_write(uint8_t *buffer, uint32_t size)
{
    // irq_set_exclusive_handler(DMA_IRQ_1, dma_handler);

    // // D'abord on va paramétrer les diverses interruptions
    // dma_channel_set_irq0_enabled(i2s->dma_ch_out_data, true);
    // irq_set_enabled(DMA_IRQ_1, true);
    // dma_channel_start(i2s->dma_ch_out_ctrl);
}
