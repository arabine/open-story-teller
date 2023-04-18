#ifndef ILI_9341_H
#define ILI_9341_H

#include <stdint.h>

void ili9341_initialize();
void ili9341_fill();
uint8_t ili9341_read_id(void);
void ili9341_shutdown();
void ili9341_set_rotation(uint8_t m);
void ili9341_draw_h_line(uint16_t y, const uint8_t *data, const uint8_t *palette);

#endif

