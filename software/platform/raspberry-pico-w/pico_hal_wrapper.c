
// OST common files
#include "ost_hal.h"
#include "debug.h"
#include "st7789.h"
#include <ff.h>
#include "diskio.h"

// Raspberry Pico SDK
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/spi.h"
#include "pico.h"

// Local
#include "pico_lcd_spi.h"
#include "pico_sdcard_spi.h"

static volatile uint32_t msTicks = 0;

// ----------------------------------------------------------------------------
// SYSTEM HAL
// ----------------------------------------------------------------------------

#define UART_ID uart0
#define BAUD_RATE 115200

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

static struct repeating_timer sys_timer;

void ost_system_delay_ms(uint32_t delay)
{
  sleep_ms(delay);
}
const uint8_t LED_PIN = 14; // GP 14

const uint8_t LCD_DC = 8;
const uint8_t LCD_CS = 9;
const uint8_t LCD_RESET = 12;
const uint8_t LCD_BL = 13;

const uint8_t ROTARY_A = 6;
const uint8_t ROTARY_B = 7;

const uint8_t SD_CARD_CS = 17;

const uint8_t SD_CARD_PRESENCE = 24;

extern void disk_timerproc();

static bool sys_timer_callback(struct repeating_timer *t)
{
  msTicks++;

  // disk_timerproc();
  return true;
}

void ost_system_initialize()
{
  // stdio_init_all();

  ////------------------- Init DEBUG LED
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  //------------------- Init UART

  // Set up our UART with the required speed.
  uart_init(UART_ID, BAUD_RATE);

  // Set the TX and RX pins by using the function select on the GPIO
  // Set datasheet for more information on function select
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

  //------------------- Init LCD

  gpio_init(LCD_DC);
  gpio_set_dir(LCD_DC, GPIO_OUT);

  gpio_init(LCD_CS);
  gpio_set_dir(LCD_CS, GPIO_OUT);

  gpio_init(LCD_RESET);
  gpio_set_dir(LCD_RESET, GPIO_OUT);

  gpio_init(LCD_BL);
  gpio_set_dir(LCD_BL, GPIO_OUT);

  pico_lcd_spi_init();

  gpio_put(LCD_RESET, 1); // enable display
  gpio_put(LCD_BL, 1);    // enable backlight

  ST7789_Init();
  ST7789_Fill_Color(MAGENTA);

  //------------------- Rotary encoder init
  gpio_init(ROTARY_A);
  gpio_set_dir(ROTARY_A, GPIO_IN);

  gpio_init(ROTARY_B);
  gpio_set_dir(ROTARY_B, GPIO_IN);

  //------------------- Init SDCARD
  gpio_init(SD_CARD_CS);
  gpio_put(SD_CARD_CS, 1);
  gpio_set_dir(SD_CARD_CS, GPIO_OUT);

  gpio_init(SD_CARD_PRESENCE);
  gpio_set_dir(SD_CARD_PRESENCE, GPIO_IN);

  pico_sdcard_spi_init(10000);

  //------------------- System timer (1ms)
  add_repeating_timer_ms(1, sys_timer_callback, NULL, &sys_timer);
}

void system_putc(char ch)
{
  uart_putc_raw(UART_ID, ch);
}

int ost_hal_gpio_get(ost_hal_gpio_t gpio)
{
  int value = 0;
  switch (gpio)
  {
  case OST_GPIO_ROTARY_A:
    value = gpio_get(ROTARY_A);
    break;
  case OST_GPIO_ROTARY_B:
    value = gpio_get(ROTARY_B);
    break;
  default:
    break;
  }

  return value;
}

void ost_hal_gpio_set(ost_hal_gpio_t gpio, int value)
{
  switch (gpio)
  {
  case OST_GPIO_DEBUG_LED:
    gpio_put(LED_PIN, value);
    break;

  // Nothing to do for these inputes
  case OST_GPIO_ROTARY_A:
  case OST_GPIO_ROTARY_B:
  default:
    break;
  }
}

// ----------------------------------------------------------------------------
// SDCARD HAL
// ----------------------------------------------------------------------------
void sdcard_set_slow_clock()
{
  // spi_init(100000, 0);
  spi_set_baudrate(spi0, 10000);
}

void sdcard_set_fast_clock()
{
  spi_set_baudrate(spi0, 1000 * 1000);
}

void ost_hal_sdcard_cs_high()
{
  gpio_put(SD_CARD_CS, 1);
}

void ost_hal_sdcard_cs_low()
{
  gpio_put(SD_CARD_CS, 0);
}

uint8_t ost_hal_sdcard_spi_transfer(uint8_t dat)
{
  uint8_t out;
  pico_ost_hal_sdcard_spi_transfer(&dat, &out, 1);

  return out;
}

uint8_t ost_hal_sdcard_get_presence()
{
  return 1; // not wired
}

// ----------------------------------------------------------------------------
// DISPLAY HAL
// ----------------------------------------------------------------------------
void ost_display_dc_high()
{
  gpio_put(LCD_DC, 1);
}

void ost_display_dc_low()
{
  gpio_put(LCD_DC, 0);
}

void ost_display_ss_high()
{
  gpio_put(LCD_CS, 1);
}

void ost_display_ss_low()
{
  gpio_put(LCD_CS, 0);
}

void ost_display_draw_h_line(uint16_t y, uint8_t *pixels, uint8_t *palette)
{
  ST7789_Fill_Line(y, pixels, palette);
}

uint8_t ost_display_transfer_byte(uint8_t dat)
{
  pico_lcd_spi_transfer(&dat, 1);
}

void ost_display_transfer_multi(uint8_t *buff, uint32_t btr)
{
  pico_lcd_spi_transfer(buff, btr);
}
