#pragma once

#include <stdint.h>

void pico_lcd_spi_init();
void pico_lcd_spi_transfer(const uint8_t *buffer, uint32_t size);
