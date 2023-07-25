

// C library
#include <string.h>
#include <stdlib.h>

// OST common files
#include "ost_hal.h"
#include "debug.h"
#include "st7789.h"
#include <ff.h>
#include "diskio.h"
#include "qor.h"

// Raspberry Pico SDK
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "pico.h"

// Local
#include "pico_lcd_spi.h"
#include "audio_player.h"
#include "pico_i2s.h"

// ===========================================================================================================
// CONSTANTS / DEFINES
// ===========================================================================================================

const uint8_t LED_PIN = 14; // GP 14

const uint8_t LCD_DC = 8;
const uint8_t LCD_CS = 9;
const uint8_t LCD_RESET = 12;
const uint8_t LCD_BL = 13;

const uint8_t ROTARY_A = 6;
const uint8_t ROTARY_B = 7;
const uint8_t ROTARY_BUTTON = 3;

#define SDCARD_SCK 18
#define SDCARD_MOSI 19
#define SDCARD_MISO 16
const uint8_t SD_CARD_CS = 17;
const uint8_t SD_CARD_PRESENCE = 24;

#define UART_ID uart0
#define BAUD_RATE 115200

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

// ===========================================================================================================
// GLOBAL VARIABLES
// ===========================================================================================================
static __attribute__((aligned(8))) pio_i2s i2s;
static volatile uint32_t msTicks = 0;
static audio_ctx_t audio_ctx;

// ===========================================================================================================
// PROTOTYPES
// ===========================================================================================================
extern void init_spi(void);
void dma_init();
void __isr __time_critical_func(audio_i2s_dma_irq_handler)();

// ===========================================================================================================
// OST HAL IMPLEMENTATION
// ===========================================================================================================

void ost_system_delay_ms(uint32_t delay)
{
  busy_wait_ms(delay);
}

void gpio_callback(uint gpio, uint32_t events)
{
  static bool one_time = true;

  // if (one_time)
  {
    one_time = false;
    // debouncer
    debug_printf("G\n");
  }
}

void ost_system_initialize()
{
  set_sys_clock_khz(125000, true);

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

  gpio_init(ROTARY_BUTTON);
  gpio_set_dir(ROTARY_BUTTON, GPIO_IN);
  gpio_disable_pulls(ROTARY_BUTTON);

  gpio_set_irq_enabled_with_callback(ROTARY_BUTTON, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

  //------------------- Init SDCARD
  gpio_init(SD_CARD_CS);
  gpio_put(SD_CARD_CS, 1);
  gpio_set_dir(SD_CARD_CS, GPIO_OUT);

  gpio_init(SD_CARD_PRESENCE);
  gpio_set_dir(SD_CARD_PRESENCE, GPIO_IN);

  spi_init(spi0, 1000 * 1000); // slow clock

  spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

  gpio_set_function(SDCARD_SCK, GPIO_FUNC_SPI);
  gpio_set_function(SDCARD_MOSI, GPIO_FUNC_SPI);
  gpio_set_function(SDCARD_MISO, GPIO_FUNC_SPI);

  //------------------- Init Sound
  static const audio_i2s_config_t config = {
      .freq = 44100,
      .bps = 32,
      .data_pin = 28,
      .clock_pin_base = 26};

  i2s_program_setup(pio0, audio_i2s_dma_irq_handler, &i2s, &config);

  audio_init(&audio_ctx);

  // ------------ Everything is initialized, print stuff here
  debug_printf("System Clock: %lu\n", clock_get_hz(clk_sys));
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
void ost_hal_sdcard_set_slow_clock()
{
  spi_set_baudrate(spi0, 10000);
}

void ost_hal_sdcard_set_fast_clock()
{
  spi_set_baudrate(spi0, 40000000UL);
}

void ost_hal_sdcard_cs_high()
{
  gpio_put(SD_CARD_CS, 1);
}

void ost_hal_sdcard_cs_low()
{
  gpio_put(SD_CARD_CS, 0);
}

void ost_hal_sdcard_spi_exchange(const uint8_t *buffer, uint8_t *out, uint32_t size)
{
  spi_write_read_blocking(spi0, buffer, out, size);
}

void ost_hal_sdcard_spi_write(const uint8_t *buffer, uint32_t size)
{
  spi_write_blocking(spi0, buffer, size);
}

void ost_hal_sdcard_spi_read(uint8_t *out, uint32_t size)
{
  spi_read_blocking(spi0, 0xFF, out, size);
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

// ----------------------------------------------------------------------------
// AUDIO HAL
// ----------------------------------------------------------------------------

void ost_audio_play(const char *filename)
{
  audio_play(&audio_ctx, filename);

  i2s.buffer_index = 0;

  // On appelle une première fois le process pour récupérer et initialiser le premier buffer...
  audio_process(&audio_ctx);

  // Puis le deuxième ... (pour avoir un buffer d'avance)
  audio_process(&audio_ctx);

  // On lance les DMA
  i2s_start(&i2s);
}

int ost_audio_process()
{
  return audio_process(&audio_ctx);
}

static ost_audio_callback_t AudioCallBack = NULL;

void ost_audio_register_callback(ost_audio_callback_t cb)
{
  AudioCallBack = cb;
}

void ost_hal_audio_new_frame(const void *buff, int dma_trans_number)
{
  if (dma_trans_number > STEREO_BUFFER_SIZE)
  {
    // Problème
    return;
  }
  memcpy(i2s.out_ctrl_blocks[i2s.buffer_index], buff, dma_trans_number * sizeof(uint32_t));
  i2s.buffer_index = 1 - i2s.buffer_index;

  //
  // dma_channel_transfer_from_buffer_now(shared_state.dma_channel, buff, dma_trans_number);
  // dma_channel_start(shared_state.dma_channel);

  //  dma_hw->ints0 = 1u << i2s.dma_ch_out_data; // clear the IRQ
}

void __isr __time_critical_func(audio_i2s_dma_irq_handler)()
{
  dma_hw->ints0 = 1u << i2s.dma_ch_out_data; // clear the IRQ

  // Warn the application layer that we have done on that channel
  if (AudioCallBack != NULL)
  {
    AudioCallBack();
  }
}

#if 0 // legacy audio test

#define SAMPLE_RATE (44100)
#define DMA_BUF_LEN (32)
#define I2S_NUM (0)
#define WAVE_FREQ_HZ (235.0f)
#define TWOPI (6.28318531f)
#define PHASE_INC (TWOPI * WAVE_FREQ_HZ / SAMPLE_RATE)

// Accumulated phase
static float p = 0.0f;

// Output buffer (2ch interleaved)
static uint32_t out_buf[DMA_BUF_LEN * 2];

uint32_t audio_count = 0;

#include <math.h>

// Fill the output buffer and write to I2S DMA
static void write_buffer()
{
  // out_buf[0] = 0x70010001;
  // out_buf[1] = 0xAAAA0001;

  dma_channel_transfer_from_buffer_now(shared_state.dma_channel, out_buf, 2);
  dma_channel_start(shared_state.dma_channel);

  // You could put a taskYIELD() here to ensure other tasks always have a chance to run.
  // taskYIELD();
}

void hal_audio_test()
{
  audio_i2s_set_enabled(true);
  audio_count = 0;
  write_buffer();
}

// irq handler for DMA
void __isr __time_critical_func(audio_i2s_dma_irq_handler)()
{
  uint dma_channel = shared_state.dma_channel;
  if (dma_irqn_get_channel_status(PICO_AUDIO_I2S_DMA_IRQ, dma_channel))
  {
    dma_irqn_acknowledge_channel(PICO_AUDIO_I2S_DMA_IRQ, dma_channel);
  }

  write_buffer();
  // busy_wait_ms(1);
  // audio_process(&audio_ctx);
}

#endif
