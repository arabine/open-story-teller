// Original file: https://github.com/dntoll/ili9341/blob/master/src/ili9341.cc

#include <stdio.h>

#include "ili9341.h"
#include "ost_hal.h"
#include "debug.h"

// FIXME: call platform independant code
#include "spi0.h"

#define ILI9341_TFTWIDTH 240  ///< ILI9341 max TFT width
#define ILI9341_TFTHEIGHT 320 ///< ILI9341 max TFT height

#define ILI9341_NOP 0x00     ///< No-op register
#define ILI9341_SWRESET 0x01 ///< Software reset register
#define ILI9341_RDDID 0x04   ///< Read display identification information
#define ILI9341_RDDST 0x09   ///< Read Display Status

#define ILI9341_SLPIN 0x10  ///< Enter Sleep Mode
#define ILI9341_SLPOUT 0x11 ///< Sleep Out
#define ILI9341_PTLON 0x12  ///< Partial Mode ON
#define ILI9341_NORON 0x13  ///< Normal Display Mode ON

#define ILI9341_RDMODE 0x0A     ///< Read Display Power Mode
#define ILI9341_RDMADCTL 0x0B   ///< Read Display MADCTL
#define ILI9341_RDPIXFMT 0x0C   ///< Read Display Pixel Format
#define ILI9341_RDIMGFMT 0x0D   ///< Read Display Image Format
#define ILI9341_RDSELFDIAG 0x0F ///< Read Display Self-Diagnostic Result

#define ILI9341_INVOFF 0x20   ///< Display Inversion OFF
#define ILI9341_INVON 0x21    ///< Display Inversion ON
#define ILI9341_GAMMASET 0x26 ///< Gamma Set
#define ILI9341_DISPOFF 0x28  ///< Display OFF
#define ILI9341_DISPON 0x29   ///< Display ON

#define ILI9341_CASET 0x2A ///< Column Address Set
#define ILI9341_PASET 0x2B ///< Page Address Set
#define ILI9341_RAMWR 0x2C ///< Memory Write
#define ILI9341_RAMRD 0x2E ///< Memory Read

#define ILI9341_PTLAR 0x30    ///< Partial Area
#define ILI9341_VSCRDEF 0x33  ///< Vertical Scrolling Definition
#define ILI9341_MADCTL 0x36   ///< Memory Access Control
#define ILI9341_VSCRSADD 0x37 ///< Vertical Scrolling Start Address
#define ILI9341_PIXFMT 0x3A   ///< COLMOD: Pixel Format Set

#define ILI9341_FRMCTR1 \
  0xB1                       ///< Frame Rate Control (In Normal Mode/Full Colors)
#define ILI9341_FRMCTR2 0xB2 ///< Frame Rate Control (In Idle Mode/8 colors)
#define ILI9341_FRMCTR3 \
  0xB3                       ///< Frame Rate control (In Partial Mode/Full Colors)
#define ILI9341_INVCTR 0xB4  ///< Display Inversion Control
#define ILI9341_DFUNCTR 0xB6 ///< Display Function Control

#define ILI9341_PWCTR1 0xC0 ///< Power Control 1
#define ILI9341_PWCTR2 0xC1 ///< Power Control 2
#define ILI9341_PWCTR3 0xC2 ///< Power Control 3
#define ILI9341_PWCTR4 0xC3 ///< Power Control 4
#define ILI9341_PWCTR5 0xC4 ///< Power Control 5
#define ILI9341_VMCTR1 0xC5 ///< VCOM Control 1
#define ILI9341_VMCTR2 0xC7 ///< VCOM Control 2
#define ILI9341_PWCTRSEQ 0xCB
#define ILI9341_PWCTRA 0xCD
#define ILI9341_PWCTRB 0xCF

#define ILI9341_RDID1 0xDA ///< Read ID 1
#define ILI9341_RDID2 0xDB ///< Read ID 2
#define ILI9341_RDID3 0xDC ///< Read ID 3
#define ILI9341_RDID4 0xDD ///< Read ID 4

#define ILI9341_GMCTRP1 0xE0
#define ILI9341_GMCTRN1 0xE1
#define ILI9341_DGMCTR1 0xE2
#define ILI9341_DGMCTR2 0xE3
#define ILI9341_TIMCTRA 0xE8
#define ILI9341_TIMCTRB 0xEA

#define ILI9341_ENGMCTR 0xF2
#define ILI9341_INCTR 0xF6
#define ILI9341_PUMP 0xF7

// Color definitions
#define ILI9341_BLACK 0x0000       ///<   0,   0,   0
#define ILI9341_NAVY 0x000F        ///<   0,   0, 123
#define ILI9341_DARKGREEN 0x03E0   ///<   0, 125,   0
#define ILI9341_DARKCYAN 0x03EF    ///<   0, 125, 123
#define ILI9341_MAROON 0x7800      ///< 123,   0,   0
#define ILI9341_PURPLE 0x780F      ///< 123,   0, 123
#define ILI9341_OLIVE 0x7BE0       ///< 123, 125,   0
#define ILI9341_LIGHTGREY 0xC618   ///< 198, 195, 198
#define ILI9341_DARKGREY 0x7BEF    ///< 123, 125, 123
#define ILI9341_BLUE 0x001F        ///<   0,   0, 255
#define ILI9341_GREEN 0x07E0       ///<   0, 255,   0
#define ILI9341_CYAN 0x07FF        ///<   0, 255, 255
#define ILI9341_RED 0xF800         ///< 255,   0,   0
#define ILI9341_MAGENTA 0xF81F     ///< 255,   0, 255
#define ILI9341_YELLOW 0xFFE0      ///< 255, 255,   0
#define ILI9341_WHITE 0xFFFF       ///< 255, 255, 255
#define ILI9341_ORANGE 0xFD20      ///< 255, 165,   0
#define ILI9341_GREENYELLOW 0xAFE5 ///< 173, 255,  41
#define ILI9341_PINK 0xFC18        ///< 255, 130, 198

static void LCD_Write_DATA(unsigned char data)
{
  ost_display_dc_high();
  spi0_cs_low();
  xchg_spi0(data);
  spi0_cs_high();
}

static void LCD_Write_COM(unsigned char com)
{
  ost_display_dc_low();
  spi0_cs_low();
  xchg_spi0(com);
  spi0_cs_high();
}

uint8_t Read_Register(uint8_t Addr, uint8_t xParameter)
{
  uint8_t data = 0;
  LCD_Write_COM(0xd9);               /* ext command                  */
  LCD_Write_DATA(0x10 + xParameter); /* 0x11 is the first Parameter  */
  ost_display_dc_low();
  spi0_cs_low();
  xchg_spi0(Addr);
  ost_display_dc_high();
  data = xchg_spi0(0);
  spi0_cs_high();
  return data;
}

static void ili9341_adressSet(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
  // https://github.com/luckasfb/lcm_drivers/blob/master/alcatel_ot_903d_jrd73_gb/lcm/ili9341/ili9341.c
  unsigned int x0 = x;
  unsigned int y0 = y;
  unsigned int x1 = x0 + width - 1;
  unsigned int y1 = y0 + height - 1;

  LCD_Write_COM(0x2a);
  LCD_Write_DATA(x0 >> 8);
  LCD_Write_DATA(x0);
  LCD_Write_DATA(x1 >> 8);
  LCD_Write_DATA(x1);

  LCD_Write_COM(0x2b);
  LCD_Write_DATA(y0 >> 8);
  LCD_Write_DATA(y0);
  LCD_Write_DATA(y1 >> 8);
  LCD_Write_DATA(y1);

  LCD_Write_COM(0x2C);
}

uint8_t ili9341_read_id(void)
{
  uint8_t i = 0;
  uint8_t data[3];
  uint8_t ID[3] = {0x00, 0x93, 0x41};
  uint8_t success = 1;

  for (i = 0; i < 3; i++)
  {
    data[i] = Read_Register(0xd3, i + 1);
    if (data[i] != ID[i])
    {
      success = 0;
    }
  }
  if (success == 0)
  {
    debug_printf("Read TFT ID failed, ID should be 0x09341, but read ID = %d %d %d\r\n",
                 (uint16_t)data[0], (uint16_t)data[1], (uint16_t)data[2]);
  }
  else
  {
    debug_printf("Detected ILI9341\r\n");
  }
  return success;
}

void ili9341_initialize()
{
  LCD_Write_COM(ILI9341_SWRESET);
  ost_system_delay_ms(120);
  LCD_Write_COM(ILI9341_DISPOFF);
  ost_system_delay_ms(120);
  LCD_Write_COM(ILI9341_PWCTRB);
  LCD_Write_DATA(0x00);
  LCD_Write_DATA(0XC1);
  LCD_Write_DATA(0X30);

  LCD_Write_COM(ILI9341_TIMCTRA);
  LCD_Write_DATA(0x85);
  LCD_Write_DATA(0x00);
  LCD_Write_DATA(0x78);

  LCD_Write_COM(ILI9341_PWCTRSEQ);
  LCD_Write_DATA(0x39);
  LCD_Write_DATA(0x2C);
  LCD_Write_DATA(0x00);
  LCD_Write_DATA(0x34);
  LCD_Write_DATA(0x02);

  LCD_Write_COM(ILI9341_PUMP);
  LCD_Write_DATA(0x20);

  LCD_Write_COM(ILI9341_TIMCTRB);
  LCD_Write_DATA(0x00);
  LCD_Write_DATA(0x00);

  LCD_Write_COM(ILI9341_PWCTR1);
  LCD_Write_DATA(0x23);

  LCD_Write_COM(ILI9341_PWCTR2);
  LCD_Write_DATA(0x10);

  LCD_Write_COM(ILI9341_VMCTR1);
  LCD_Write_DATA(0x3e);
  LCD_Write_DATA(0x28);

  LCD_Write_COM(ILI9341_VMCTR2);
  LCD_Write_DATA(0x86);

  LCD_Write_COM(ILI9341_MADCTL);
  LCD_Write_DATA(0x48);

  LCD_Write_COM(ILI9341_PIXFMT);
  LCD_Write_DATA(0x55);

  LCD_Write_COM(ILI9341_FRMCTR1);
  LCD_Write_DATA(0x00);
  LCD_Write_DATA(0x18);

  LCD_Write_COM(ILI9341_DFUNCTR);
  LCD_Write_DATA(0x08);
  LCD_Write_DATA(0x82);
  LCD_Write_DATA(0x27);

  LCD_Write_COM(ILI9341_ENGMCTR);
  LCD_Write_DATA(0x00);

  LCD_Write_COM(ILI9341_GAMMASET);
  LCD_Write_DATA(0x01);

  LCD_Write_COM(ILI9341_GMCTRP1);
  LCD_Write_DATA(0x0F);
  LCD_Write_DATA(0x31);
  LCD_Write_DATA(0x2B);
  LCD_Write_DATA(0x0C);
  LCD_Write_DATA(0x0E);
  LCD_Write_DATA(0x08);
  LCD_Write_DATA(0x4E);
  LCD_Write_DATA(0xF1);
  LCD_Write_DATA(0x37);
  LCD_Write_DATA(0x07);
  LCD_Write_DATA(0x10);
  LCD_Write_DATA(0x03);
  LCD_Write_DATA(0x0E);
  LCD_Write_DATA(0x09);
  LCD_Write_DATA(0x00);

  LCD_Write_COM(ILI9341_GMCTRN1);
  LCD_Write_DATA(0x00);
  LCD_Write_DATA(0x0E);
  LCD_Write_DATA(0x14);
  LCD_Write_DATA(0x03);
  LCD_Write_DATA(0x11);
  LCD_Write_DATA(0x07);
  LCD_Write_DATA(0x31);
  LCD_Write_DATA(0xC1);
  LCD_Write_DATA(0x48);
  LCD_Write_DATA(0x08);
  LCD_Write_DATA(0x0F);
  LCD_Write_DATA(0x0C);
  LCD_Write_DATA(0x31);
  LCD_Write_DATA(0x36);
  LCD_Write_DATA(0x0F);

  LCD_Write_COM(ILI9341_SLPOUT);
  ost_system_delay_ms(120);
  LCD_Write_COM(ILI9341_DISPON);
}

void ili9341_shutdown()
{
  // https://github.com/fernando-rodriguez/lglib/blob/db2e2cad07264b8fce224d15fd5080675fbc4c89/ili9341/ili9341.c
  LCD_Write_COM(0x10); // ENTER SLEEP MODE
  LCD_Write_COM(0x28); // DISPLAY OFF
}

#define MADCTL_MY 0x80  ///< Bottom to top
#define MADCTL_MX 0x40  ///< Right to left
#define MADCTL_MV 0x20  ///< Reverse Mode
#define MADCTL_ML 0x10  ///< LCD refresh Bottom to top
#define MADCTL_RGB 0x00 ///< Red-Green-Blue pixel order
#define MADCTL_BGR 0x08 ///< Blue-Green-Red pixel order
#define MADCTL_MH 0x04  ///< LCD refresh right to left

void ili9341_set_rotation(uint8_t m)
{
  uint8_t rotation = m & 3; // can't be higher than 3
  switch (rotation)
  {
  case 0:
    m = (MADCTL_MX | MADCTL_BGR);
    break;
  case 1:
    m = (MADCTL_MV | MADCTL_BGR);
    break;
  case 2:
    m = (MADCTL_MY | MADCTL_BGR);
    break;
  case 3:
    m = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
    break;
  }

  LCD_Write_COM(ILI9341_MADCTL);
  LCD_Write_DATA(m);
}

static uint16_t WIDTH = 320;
static uint16_t HEIGHT = 240;

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

void ili9341_draw_h_line(uint16_t y, const uint8_t *data, const uint8_t *palette)
{
  ili9341_adressSet(0, y, 320, 240);
  ost_display_dc_high();
  spi0_cs_low();

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
    uint16_t pixel = color565(color.r, color.g, color.b);

    xchg_spi0(color_get_high(&color));
    xchg_spi0(color_get_low(&color));

    // pixel = ILI9341_PURPLE;
    // xchg_spi0(pixel>>8);
    // xchg_spi0(pixel&0xFF);
  }

  spi0_cs_high();
}

void ili9341_fill()
{
  uint16_t pixel = ILI9341_PURPLE;
  uint16_t pixel2 = ILI9341_YELLOW;
  int line = 0;

  for (uint16_t j = 0; j < 240; j++)
  {
    ili9341_adressSet(0, j, 320, 240);
    ost_display_dc_high();
    spi0_cs_low();
    uint8_t hi = pixel >> 8;
    uint8_t lo = pixel;

    if (line == 0)
    {
      line = 1;
    }
    else
    {
      line = 0;
      hi = pixel2 >> 8;
      lo = pixel2;
    }

    for (uint16_t i = 0; i < 320; i++)
    {
      xchg_spi0(hi);
      xchg_spi0(lo);
    }

    spi0_cs_high();
  }
}
