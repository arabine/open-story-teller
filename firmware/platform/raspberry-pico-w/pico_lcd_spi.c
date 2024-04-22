
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/dma.h"

#define LCD_SCK 10
#define LCD_TX 11

static uint dma_tx;

void pico_lcd_spi_init()
{
    spi_init(spi1, 10000000);
    // gpio_set_function(LCD_RX, GPIO_FUNC_SPI);

    gpio_set_function(LCD_SCK, GPIO_FUNC_SPI);
    gpio_set_function(LCD_TX, GPIO_FUNC_SPI);

    // Grab some unused dma channels
    // dma_tx = dma_claim_unused_channel(true);
    // uint dma_rx = dma_claim_unused_channel(true);

    // Force loopback for testing (I don't have an SPI device handy)
    //    hw_set_bits(&spi_get_hw(spi_default)->cr1, SPI_SSPCR1_LBM_BITS);
    /*
        static uint8_t txbuf[TEST_SIZE];
        static uint8_t rxbuf[TEST_SIZE];
        for (uint i = 0; i < TEST_SIZE; ++i)
        {
            txbuf[i] = rand();
        }
        */
}

void pico_lcd_spi_transfer(const uint8_t *buffer, uint32_t size)
{
    spi_write_blocking(spi1, buffer, size);
    /* ------- DMA
    // We set the outbound DMA to transfer from a memory buffer to the SPI transmit FIFO paced by the SPI TX FIFO DREQ
    // The default is for the read address to increment every element (in this case 1 byte = DMA_SIZE_8)
    // and for the write address to remain unchanged.

    printf("Configure TX DMA\n");
    dma_channel_config c = dma_channel_get_default_config(dma_tx);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(spi_default, true));
    dma_channel_configure(dma_tx, &c,
                          &spi_get_hw(spi_default)->dr, // write address
                          buffer,                       // read address
                          size,                         // element count (each element is of size transfer_data_size)
                          false);                       // don't start yet

    dma_start_channel_mask((1u << dma_tx));
    dma_channel_wait_for_finish_blocking(dma_tx);

    */
}
