#include "st7789.h"
#include "ost_hal.h"
#include "debug.h"

/**
 * @brief Write command to ST7789 controller
 * @param cmd -> command to write
 * @return none
 */
static void ST7789_WriteCommand(uint8_t cmd)
{
	ost_display_dc_low();
	ost_display_ss_low();
	ost_display_transfer_byte(cmd);
	// HAL_SPI_Transmit(&ST7789_SPI_PORT, &cmd, sizeof(cmd), HAL_MAX_DELAY);
	ost_display_ss_high();
}

/**
 * @brief Write data to ST7789 controller
 * @param buff -> pointer of data buffer
 * @param buff_size -> size of the data buffer
 * @return none
 */
static void ST7789_WriteData(uint8_t *buff, uint32_t buff_size)
{
	ost_display_dc_high();
	ost_display_ss_low();

	// split data in small chunks because HAL can't send more than 64K at once

	while (buff_size > 0)
	{
		uint16_t chunk_size = buff_size > 65535 ? 65535 : buff_size;
		ost_display_transfer_multi(buff, chunk_size);
		// HAL_SPI_Transmit(&ST7789_SPI_PORT, buff, chunk_size, HAL_MAX_DELAY);

		buff += chunk_size;
		buff_size -= chunk_size;
	}

	ost_display_ss_high();
}
/**
 * @brief Write data to ST7789 controller, simplify for 8bit data.
 * data -> data to write
 * @return none
 */
static void ST7789_WriteSmallData(uint8_t data)
{
	ost_display_dc_high();
	ost_display_ss_low();

	// HAL_SPI_Transmit(&ST7789_SPI_PORT, &data, sizeof(data), HAL_MAX_DELAY);
	ost_display_transfer_byte(data);
	ost_display_ss_high();
}

/**
 * @brief Set the rotation direction of the display
 * @param m -> rotation parameter(please refer it in st7789.h)
 * @return none
 */
void ST7789_SetRotation(uint8_t m)
{
	ST7789_WriteCommand(ST7789_MADCTL); // MADCTL
	switch (m)
	{
	case 0:
		ST7789_WriteSmallData(ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_RGB);
		break;
	case 1:
		ST7789_WriteSmallData(ST7789_MADCTL_MY | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);
		break;
	case 2:
		ST7789_WriteSmallData(ST7789_MADCTL_RGB);
		break;
	case 3:
		ST7789_WriteSmallData(ST7789_MADCTL_MX | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);
		break;
	default:
		break;
	}
}

/**
 * @brief Set address of DisplayWindow
 * @param xi&yi -> coordinates of window
 * @return none
 */
static void ST7789_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	ost_display_ss_low();
	uint16_t x_start = x0 + X_SHIFT, x_end = x1 + X_SHIFT;
	uint16_t y_start = y0 + Y_SHIFT, y_end = y1 + Y_SHIFT;

	/* Column Address set */
	ST7789_WriteCommand(ST7789_CASET);
	{
		uint8_t data[] = {x_start >> 8, x_start & 0xFF, x_end >> 8, x_end & 0xFF};
		ST7789_WriteData(data, sizeof(data));
	}

	/* Row Address set */
	ST7789_WriteCommand(ST7789_RASET);
	{
		uint8_t data[] = {y_start >> 8, y_start & 0xFF, y_end >> 8, y_end & 0xFF};
		ST7789_WriteData(data, sizeof(data));
	}
	/* Write to RAM */
	ST7789_WriteCommand(ST7789_RAMWR);
	ost_display_ss_high();
}

/**
 * @brief Initialize ST7789 controller
 * @param none
 * @return none
 */
void ST7789_Init(void)
{
#ifdef USE_DMA
	memset(disp_buf, 0, sizeof(disp_buf));
#endif
	// ost_system_delay_ms(25);
	// ST7789_RST_Clr();
	// ost_system_delay_ms(25);
	// ST7789_RST_Set();
	// ost_system_delay_ms(50);

	ST7789_WriteCommand(0x28); // Turn off display
	ST7789_WriteCommand(0x11); // Exit sleep mode
	ost_system_delay_ms(100);
	ST7789_WriteCommand(0x01); // Software Reset
	ost_system_delay_ms(150);

	// Does not word, read 00 42 C2 FF ?? (should be 00 85 85 52 )
	ost_display_dc_low();
	ost_display_ss_low();
	ost_display_transfer_byte(ST7789_RDDID);
	uint8_t buff[4] = {0xFF, 0xFF, 0xFF, 0xFF};
	ost_display_transfer_multi(buff, sizeof(buff));

	for (int i = 0; i < sizeof(buff); i++)
	{
		debug_printf("%X ", buff[i]);
	}
	ost_display_ss_high();

	ST7789_SetRotation(ST7789_ROTATION); //	MADCTL (Display Rotation)

	// ST7789_WriteCommand(ST7789_MADCTL);	// MADCTL
	// ST7789_WriteSmallData(0x88);

	ST7789_WriteCommand(ST7789_COLMOD); //	Set color mode
	ST7789_WriteSmallData(ST7789_COLOR_MODE_16bit);
	ST7789_WriteCommand(0xB2); //	Porch control
	{
		uint8_t data[] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
		ST7789_WriteData(data, sizeof(data));
	}

	/* Internal LCD Voltage generator settings */
	ST7789_WriteCommand(0XB7);	 //	Gate Control
	ST7789_WriteSmallData(0x35); //	Default value
	ST7789_WriteCommand(0xBB);	 //	VCOM setting
	ST7789_WriteSmallData(0x2B); //	0.725v (default 0.75v for 0x20)
	ST7789_WriteCommand(0xC0);	 //	LCMCTRL
	ST7789_WriteSmallData(0x2C); //	Default value

	ST7789_WriteCommand(0xC2); //	VDV and VRH command Enable
	{
		uint8_t data[] = {0x01, 0xFF};
		ST7789_WriteData(data, sizeof(data));
	}

	ST7789_WriteCommand(0xC3);	 //	VRH set
	ST7789_WriteSmallData(0x11); //	+-4.45v (defalut +-4.1v for 0x0B)

	ST7789_WriteCommand(0xC4);	 //	VDV set
	ST7789_WriteSmallData(0x20); //	Default value

	ST7789_WriteCommand(0xC6);	 //	Frame rate control in normal mode
	ST7789_WriteSmallData(0x0F); //	Default value (60HZ)

	ST7789_WriteCommand(0xD0);	 //	Power control
	ST7789_WriteSmallData(0xA4); //	Default value
	ST7789_WriteSmallData(0xA1); //	Default value
	/**************** Division line ****************/

	ST7789_WriteCommand(0xE0);
	{
		uint8_t data[] = {0xD0, 0x00, 0x05, 0x0E, 0x15, 0x0D, 0x37, 0x43, 0x47, 0x09, 0x15, 0x12, 0x16, 0x19};
		ST7789_WriteData(data, sizeof(data));
	}

	ST7789_WriteCommand(0xE1);
	{
		uint8_t data[] = {0xD0, 0x00, 0x05, 0x0D, 0x0C, 0x06, 0x2D, 0x44, 0x40, 0x0E, 0x1C, 0x18, 0x16, 0x19};
		ST7789_WriteData(data, sizeof(data));
	}
	ST7789_WriteCommand(ST7789_INVOFF); //	Inversion OFF
	ST7789_WriteCommand(ST7789_SLPOUT); //	Out of sleep mode
	ST7789_WriteCommand(ST7789_NORON);	//	Normal Display on
	ST7789_WriteCommand(ST7789_DISPON); //	Main screen turned on

	ost_system_delay_ms(50);
	//	ST7789_Fill_Color(BLACK);				//	Fill with Black.
}

// rrrrrggggggbbbbb
uint8_t color_get_high(const color_t *c)
{
	return ((c->r & 248) | (c->g >> 5));
}
// rrrrrggggggbbbbb
uint8_t color_get_low(const color_t *c)
{
	return ((c->g & 28) << 3 | (c->b >> 3));
}

uint16_t color565(uint8_t r, uint8_t g, uint8_t b)
{
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void ST7789_Fill_Line(uint16_t y, const uint8_t *data, const uint8_t *palette)
{
	// ili9341_adressSet(0, y, 320, 240);
	ST7789_SetAddressWindow(0, y, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);
	ost_display_dc_high();
	ost_display_ss_low();

	color_t color;

	for (uint16_t i = 0; i < 320; i++)
	{
		uint8_t val = data[i];

		if (val > 15)
		{
			val = 0;
			debug_printf("***");
		}

		const uint8_t *palettePtr = &palette[val * 4];
		color.r = palettePtr[0];
		color.g = palettePtr[1];
		color.b = palettePtr[2];
		//   uint16_t pixel = color565(color.r, color.g, color.b);

		ost_display_transfer_byte(color_get_high(&color));
		ost_display_transfer_byte(color_get_low(&color));

		// pixel = ILI9341_PURPLE;
		// xchg_spi0(pixel>>8);
		// xchg_spi0(pixel&0xFF);
	}

	ost_display_ss_high();
}

/**
 * @brief Fill the DisplayWindow with single color
 * @param color -> color to Fill with
 * @return none
 */
void ST7789_Fill_Color(uint16_t color)
{
	uint16_t i;
	ST7789_SetAddressWindow(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);
	ost_display_ss_low();

#ifdef USE_DMA
	for (i = 0; i < ST7789_HEIGHT / HOR_LEN; i++)
	{
		memset(disp_buf, color, sizeof(disp_buf));
		ST7789_WriteData(disp_buf, sizeof(disp_buf));
	}
#else
	uint16_t j;
	for (i = 0; i < ST7789_WIDTH; i++)
		for (j = 0; j < ST7789_HEIGHT; j++)
		{
			uint8_t data[] = {color >> 8, color & 0xFF};
			ST7789_WriteData(data, sizeof(data));
		}
#endif
	ost_display_ss_high();
}

/**
 * @brief Draw a Pixel
 * @param x&y -> coordinate to Draw
 * @param color -> color of the Pixel
 * @return none
 */
void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
	if ((x < 0) || (x >= ST7789_WIDTH) ||
		(y < 0) || (y >= ST7789_HEIGHT))
		return;

	ST7789_SetAddressWindow(x, y, x, y);
	uint8_t data[] = {color >> 8, color & 0xFF};
	ost_display_ss_low();
	ST7789_WriteData(data, sizeof(data));
	ost_display_ss_high();
}

/**
 * @brief Fill an Area with single color
 * @param xSta&ySta -> coordinate of the start point
 * @param xEnd&yEnd -> coordinate of the end point
 * @param color -> color to Fill with
 * @return none
 */
void ST7789_Fill(uint16_t xSta, uint16_t ySta, uint16_t xEnd, uint16_t yEnd, uint16_t color)
{
	if ((xEnd < 0) || (xEnd >= ST7789_WIDTH) ||
		(yEnd < 0) || (yEnd >= ST7789_HEIGHT))
		return;
	ost_display_ss_low();
	uint16_t i, j;
	ST7789_SetAddressWindow(xSta, ySta, xEnd, yEnd);
	for (i = ySta; i <= yEnd; i++)
		for (j = xSta; j <= xEnd; j++)
		{
			uint8_t data[] = {color >> 8, color & 0xFF};
			ST7789_WriteData(data, sizeof(data));
		}
	ost_display_ss_high();
}

/**
 * @brief Draw a big Pixel at a point
 * @param x&y -> coordinate of the point
 * @param color -> color of the Pixel
 * @return none
 */
void ST7789_DrawPixel_4px(uint16_t x, uint16_t y, uint16_t color)
{
	if ((x <= 0) || (x > ST7789_WIDTH) ||
		(y <= 0) || (y > ST7789_HEIGHT))
		return;
	ost_display_ss_low();
	ST7789_Fill(x - 1, y - 1, x + 1, y + 1, color);
	ost_display_ss_high();
}

/**
 * @brief Draw an Image on the screen
 * @param x&y -> start point of the Image
 * @param w&h -> width & height of the Image to Draw
 * @param data -> pointer of the Image array
 * @return none
 */
void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data)
{
	if ((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT))
		return;
	if ((x + w - 1) >= ST7789_WIDTH)
		return;
	if ((y + h - 1) >= ST7789_HEIGHT)
		return;

	ost_display_ss_low();
	ST7789_SetAddressWindow(x, y, x + w - 1, y + h - 1);
	ST7789_WriteData((uint8_t *)data, sizeof(uint16_t) * w * h);
	ost_display_ss_high();
}

/**
 * @brief Invert Fullscreen color
 * @param invert -> Whether to invert
 * @return none
 */
void ST7789_InvertColors(uint8_t invert)
{
	ost_display_ss_low();
	ST7789_WriteCommand(invert ? 0x21 /* INVON */ : 0x20 /* INVOFF */);
	ost_display_ss_high();
}

/*
void ST7789_Test(void)
{
	ST7789_Fill_Color(WHITE);
	ost_system_delay_ms(1000);
	ST7789_WriteString(10, 20, "Speed Test", Font_11x18, RED, WHITE);
	ost_system_delay_ms(1000);
	ST7789_Fill_Color(CYAN);
	ost_system_delay_ms(500);
	ST7789_Fill_Color(RED);
	ost_system_delay_ms(500);
	ST7789_Fill_Color(BLUE);
	ost_system_delay_ms(500);
	ST7789_Fill_Color(GREEN);
	ost_system_delay_ms(500);
	ST7789_Fill_Color(YELLOW);
	ost_system_delay_ms(500);
	ST7789_Fill_Color(BROWN);
	ost_system_delay_ms(500);
	ST7789_Fill_Color(DARKBLUE);
	ost_system_delay_ms(500);
	ST7789_Fill_Color(MAGENTA);
	ost_system_delay_ms(500);
	ST7789_Fill_Color(LIGHTGREEN);
	ost_system_delay_ms(500);
	ST7789_Fill_Color(LGRAY);
	ost_system_delay_ms(500);
	ST7789_Fill_Color(LBBLUE);
	ost_system_delay_ms(500);
	ST7789_Fill_Color(WHITE);
	ost_system_delay_ms(500);

	ST7789_WriteString(10, 10, "Font test.", Font_16x26, GBLUE, WHITE);
	ST7789_WriteString(10, 50, "Hello Steve!", Font_7x10, RED, WHITE);
	ST7789_WriteString(10, 75, "Hello Steve!", Font_11x18, YELLOW, WHITE);
	ST7789_WriteString(10, 100, "Hello Steve!", Font_16x26, MAGENTA, WHITE);
	ost_system_delay_ms(1000);

	ST7789_Fill_Color(RED);
	ST7789_WriteString(10, 10, "Rect./Line.", Font_11x18, YELLOW, BLACK);
	ST7789_DrawRectangle(30, 30, 100, 100, WHITE);
	ost_system_delay_ms(1000);

	ST7789_Fill_Color(RED);
	ST7789_WriteString(10, 10, "Filled Rect.", Font_11x18, YELLOW, BLACK);
	ST7789_DrawFilledRectangle(30, 30, 50, 50, WHITE);
	ost_system_delay_ms(1000);

	ST7789_Fill_Color(RED);
	ST7789_WriteString(10, 10, "Circle.", Font_11x18, YELLOW, BLACK);
	ST7789_DrawCircle(60, 60, 25, WHITE);
	ost_system_delay_ms(1000);

	ST7789_Fill_Color(RED);
	ST7789_WriteString(10, 10, "Filled Cir.", Font_11x18, YELLOW, BLACK);
	ST7789_DrawFilledCircle(60, 60, 25, WHITE);
	ost_system_delay_ms(1000);

	ST7789_Fill_Color(RED);
	ST7789_WriteString(10, 10, "Triangle", Font_11x18, YELLOW, BLACK);
	ST7789_DrawTriangle(30, 30, 30, 70, 60, 40, WHITE);
	ost_system_delay_ms(1000);

	ST7789_Fill_Color(RED);
	ST7789_WriteString(10, 10, "Filled Tri", Font_11x18, YELLOW, BLACK);
	ST7789_DrawFilledTriangle(30, 30, 30, 70, 60, 40, WHITE);
	ost_system_delay_ms(1000);

	//	If FLASH cannot storage anymore datas, please delete codes below.
	ST7789_Fill_Color(WHITE);
	ST7789_DrawImage(0, 0, 128, 128, (uint16_t *)saber);
	ost_system_delay_ms(3000);
}
*/