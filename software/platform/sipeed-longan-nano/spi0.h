
#ifndef SPI0_DEFINED
#define SPI0_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
SD Cards operate at two speed modes. The default mode clock speed is 0 -25MHz. A high speed mode is available at clock speed of 0 -50MHz.
*/
void spi0_set_fclk_slow();
void spi0_set_fclk_fast();

void spi0_cs_high();
void spi0_cs_low();


uint8_t xchg_spi0 (uint8_t dat);


/**
 * Setup SPI block ID
 */
void spi0_initialize();


#endif // SPI_DEFINED
