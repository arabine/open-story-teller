
#ifndef SPI_DEFINED
#define SPI_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
SD Cards operate at two speed modes. The default mode clock speed is 0 -25MHz. A high speed mode is available at clock speed of 0 -50MHz.
*/
void spi1_set_fclk_slow();
void spi1_set_fclk_fast();

void spi1_cs_high();
void spi1_cs_low();


uint8_t xchg_spi1 (uint8_t dat);

void spi1_initialize();


#endif // SPI_DEFINED
