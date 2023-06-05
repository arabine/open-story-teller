#pragma once

#include <stdint.h>

void pico_sdcard_spi_init(int clock);
void pico_ost_hal_sdcard_spi_transfer(const uint8_t *buffer, uint8_t *out, uint32_t size);
