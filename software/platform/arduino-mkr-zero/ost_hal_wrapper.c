#include "ost_hal.h"

#include "samd21.h"
#include "uart.h"
#include "hal_gpio.h"
#include "spi_master.h"
#include "uart.h"

#include "string.h"
#include "debug.h"
#include "st7789.h"
#include "spi_display.h"

#include "arduino_platform.h"

static volatile uint32_t msTicks = 0;

//-----------------------------------------------------------------------------
#define PERIOD_FAST 1200
#define PERIOD_SLOW 500

HAL_GPIO_PIN(LED, B, 8) // Built-in LED
HAL_GPIO_PIN(BUTTON, A, 15)

HAL_GPIO_PIN(CD, A, 27);

//-----------------------------------------------------------------------------
void timer_set_period(uint16_t i)
{
  TC3->COUNT16.CC[0].reg = ((F_CPU / 1000ul) * i) / 256;
  TC3->COUNT16.COUNT.reg = 0;
}

//-----------------------------------------------------------------------------
void ost_tasker_timer_callback();

void TC3_Handler(void)
{
  if (TC3->COUNT16.INTFLAG.reg & TC_INTFLAG_MC(1))
  {
    // HAL_GPIO_LED_toggle();
    ost_tasker_timer_callback();
    TC3->COUNT16.INTFLAG.reg = TC_INTFLAG_MC(1);
  }
}

// Setup TC4/TC5 in 32-bit mode with 100kHz tick
void timer_32_init()
{

  GCLK->GENDIV.reg = GCLK_GENDIV_DIV(30) | // Divide the 48MHz system clock by 3 = 1.6MHz
                     GCLK_GENDIV_ID(5);    // Set division on Generic Clock Generator (GCLK) 5
  while (GCLK->STATUS.bit.SYNCBUSY)
    ; // Wait for synchronization

  GCLK->GENCTRL.reg = GCLK_GENCTRL_IDC |         // Set the duty cycle to 50/50 HIGH/LOW
                      GCLK_GENCTRL_GENEN |       // Enable GCLK 5
                      GCLK_GENCTRL_SRC_DFLL48M | // Set the clock source to 48MHz
                      GCLK_GENCTRL_ID(5);        // Set clock source on GCLK 5
  while (GCLK->STATUS.bit.SYNCBUSY)
    ; // Wait for synchronization

  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |     // Enable the generic clock...
                      GCLK_CLKCTRL_GEN_GCLK5 | // ....on GCLK5
                      GCLK_CLKCTRL_ID_TC4_TC5; // Feed the GCLK5 to TC4 and TC5
  while (GCLK->STATUS.bit.SYNCBUSY)
    ; // Wait for synchronization

  TC4->COUNT32.CTRLA.reg |= TC_CTRLA_PRESCALER_DIV16 | // Set prescaler to 16, 1.6MHz/16 = 100kHz
                            TC_CTRLA_PRESCSYNC_PRESC | // Reload timer on next prescaler clock
                            TC_CTRLA_MODE_COUNT32;     // Set the TC4 timer to 32-bit mode in conjuction with timer TC5

  TC4->COUNT32.CTRLA.bit.ENABLE = 1; // Enable TC4
  while (TC4->COUNT32.STATUS.bit.SYNCBUSY)
    ; // Wait for synchronization

  TC4->COUNT32.READREQ.reg = TC_READREQ_RCONT |     // Enable a continuous read request
                             TC_READREQ_ADDR(0x10); // Offset of the 32-bit COUNT register
  while (TC4->COUNT32.STATUS.bit.SYNCBUSY)
    ; // Wait for (read) synchronization
}

//-----------------------------------------------------------------------------
static void timer_init(void)
{
  PM->APBCMASK.reg |= PM_APBCMASK_TC3;

  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(TC3_GCLK_ID) |
                      GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(0);

  TC3->COUNT16.CTRLA.reg = TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ |
                           TC_CTRLA_PRESCALER_DIV256 | TC_CTRLA_PRESCSYNC_RESYNC;

  TC3->COUNT16.COUNT.reg = 0;

  // timer_set_period(PERIOD_SLOW);

  TC3->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;

  TC3->COUNT16.INTENSET.reg = TC_INTENSET_MC(1);
  NVIC_EnableIRQ(TC3_IRQn);
}

void arduino_mkrzero_init(void);

// ----------------------------------------------------------------------------
// SYSTEM HAL
// ----------------------------------------------------------------------------

#if 0
// Cortex-M0 does not have DWT register accessible from CPU

// Register base address
#define DWT_CR *(uint32_t *)0xE0001000
#define DWT_CYCCNT *(uint32_t *)0xE0001004
#define DEM_CR *(uint32_t *)0xE000EDFC
 
// Define the required enable bit
#define DEM_CR_TRCENA (1 << 24)
#define DWT_CR_CYCCNTENA (1 << 0)


static void cortex_m_cycle_counter_init()
{
  DEM_CR |= (uint32_t)DEM_CR_TRCENA;
  DWT_CYCCNT = (uint32_t)0u;
  DWT_CR |= (uint32_t)DWT_CR_CYCCNTENA;
}

static uint32_t cortex_m_cycle_get_counter()
{
    return (uint32_t)DWT_CYCCNT;
}

static void cortex_m_delay_ms(volatile uint32_t milliseconds)
{
  uint32_t au32_initial_ticks = cortex_m_cycle_get_counter();
  uint32_t au32_ticks = (F_CPU / 1000000);
  milliseconds *= au32_ticks;
  while ((cortex_m_cycle_get_counter() - au32_initial_ticks) < milliseconds);
}
#endif

// Set up RTC for counting
void setupRTC()
{
  // configure the 32768 Hz oscillator
  // SYSCTRL->XOSC32K.reg =  SYSCTRL_XOSC32K_ONDEMAND |
  //                         SYSCTRL_XOSC32K_RUNSTDBY |
  //                         SYSCTRL_XOSC32K_EN32K |
  //                         SYSCTRL_XOSC32K_XTALEN |
  //                         SYSCTRL_XOSC32K_STARTUP(6) |
  //                         SYSCTRL_XOSC32K_ENABLE;

  // attach peripheral clock to 32768 Hz oscillator (1 tick = 1/32768 second)
  // GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) | GCLK_GENDIV_DIV(0);
  // GCLK->GENCTRL.reg = (GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_XOSC32K | GCLK_GENCTRL_ID(2));
  GCLK->CLKCTRL.reg = (uint32_t)((GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK5 | (RTC_GCLK_ID << GCLK_CLKCTRL_ID_Pos)));

  // disable RTC if it is enabled, to allow reconfiguration
  RTC->MODE0.CTRL.reg &= ~RTC_MODE0_CTRL_ENABLE;

  // trigger RTC software reset
  RTC->MODE0.CTRL.reg |= RTC_MODE0_CTRL_SWRST;

  // configure RTC in mode 0 (32-bit)
  RTC->MODE0.CTRL.reg |= RTC_MODE0_CTRL_PRESCALER_DIV1 | RTC_MODE0_CTRL_MODE_COUNT32;

  // set match_clear bit(7) to clear counter for periodic interrupts
  // this will probably screw up anything else using the RTC
  // See Table 18-1. MODE0 - Mode Register Summary
  RTC->MODE0.CTRL.reg |= RTC_MODE0_CTRL_MATCHCLR;

  // enter freq correction here as sign (bit7) and magnitude (bits 6-0)
  RTC_FREQCORR_VALUE(0x00); // adjust if necessary

  // initialize counter & compare values
  RTC->MODE0.COUNT.reg = 0;
  RTC->MODE0.COMP[0].reg = 32768 * 5; // 32768 * 2 seconds

  // enable the CMP0 interrupt in the RTC
  RTC->MODE0.INTENSET.reg |= RTC_MODE0_INTENSET_CMP0;

  // re-enable RTC after reconfiguration and initial scheduling
  RTC->MODE0.CTRL.reg |= RTC_MODE0_CTRL_ENABLE;

  // enable RTC interrupt in controller
  NVIC_SetPriority(RTC_IRQn, 0x00);
  NVIC_EnableIRQ(RTC_IRQn);

  // change USB/EIC priority to 0x01 to prevent preempting the RTC
  // NVIC_SetPriority(USB_IRQn, 0x01);
  // NVIC_SetPriority(EIC_IRQn, 0x01);

  // enable continuous synchronization
  while (RTC->MODE0.STATUS.bit.SYNCBUSY)
    ;
  RTC->MODE0.READREQ.reg = RTC_READREQ_RREQ | RTC_READREQ_RCONT | 0x0010;
  while (RTC->MODE0.STATUS.bit.SYNCBUSY)
    ;
} // end setupRTC

void RTC_Handler(void)
{
  if (RTC->MODE0.INTFLAG.bit.CMP0)
  {
    HAL_GPIO_LED_toggle();
    RTC->MODE0.INTFLAG.reg = 0xFF; // Clear all interrupts
    // RTC->MODE0.INTFLAG.reg |= 0x81; // Clear CMP0 and OVF interrupts only
  }
}

void ost_system_delay_ms(uint32_t delay)
{
  uint32_t t0 = TC4->COUNT32.COUNT.reg;

  delay *= 100;
  bool quit = false;
  do
  {
    uint32_t t1 = TC4->COUNT32.COUNT.reg;
    uint32_t diff = t1 - t0;
    quit = diff >= delay;
  } while (!quit);
}

void ost_system_initialize()
{
  arduino_mkrzero_init();

  // cortex_m_cycle_counter_init();

  // SysTick_Config(F_CPU / 1000);

  //  timer_init();
  timer_32_init();
  uart_init(115200);

  HAL_GPIO_LED_out();
  HAL_GPIO_LED_clr();

  //  timer_set_period(PERIOD_FAST);

  // Initialize SPI for SD Card
  spi_init(250000, 0);

  // SPI for display
  spi_display_init(10000000, 0);

  HAL_GPIO_CD_in();
  HAL_GPIO_CD_pullup();

  //   setupRTC();

  //   sd_card_test();

  /*
    uart_puts("\r\nHello, world!\r\n");

    HAL_GPIO_LED_out();
    HAL_GPIO_LED_clr();

    HAL_GPIO_BUTTON_in();
    HAL_GPIO_BUTTON_pullup();

    while (1)
    {
      if (HAL_GPIO_BUTTON_read())
        cnt = 0;
      else if (cnt < 5001)
        cnt++;

      if (5000 == cnt)
      {
        fast = !fast;
        timer_set_period(fast ? PERIOD_FAST : PERIOD_SLOW);
        uart_putc('.');
      }
    }*/
}

void system_putc(char ch)
{
  uart_putc(ch);
}

// void SysTick_Handler()
// {
//     msTicks++;
// }

void system_led_write(uint8_t value)
{
  HAL_GPIO_LED_write(value);
}

// ----------------------------------------------------------------------------
// SDCARD HAL
// ----------------------------------------------------------------------------
void sdcard_set_slow_clock()
{
  spi_init(100000, 0);
}

void sdcard_set_fast_clock()
{
  spi_init(800000, 0);
}

void sdcard_cs_high()
{
  spi_ss(1);
}

void sdcard_cs_low()
{
  spi_ss(0);
}

uint8_t sdcard_spi_transfer(uint8_t dat)
{
  return spi_transfer(dat);
}

void sdcard_spi_recv_multi(uint8_t *buff, uint32_t btr)
{
  for (uint32_t i = 0; i < btr; i++)
  {
    buff[i] = spi_transfer(buff[i]);
  }
}

// ----------------------------------------------------------------------------
// DISPLAY HAL
// ----------------------------------------------------------------------------
#define DC_PIN GPIO_PIN_1
#define DC_GPIO_PORT GPIOA
#define DC_GPIO_CLK RCU_GPIOA

void ost_display_initialize()
{
  ST7789_Init();
  ST7789_Fill_Color(MAGENTA);
}

void ost_display_dc_high()
{
  spi_display_dc(1);
}

void ost_display_dc_low()
{
  spi_display_dc(0);
}

void ost_display_ss_high()
{
  spi_display_ss(1);
}

void ost_display_ss_low()
{
  spi_display_ss(0);
}

void ost_display_draw_h_line(uint16_t y, uint8_t *pixels, uint8_t *palette)
{
  ST7789_Fill_Line(y, pixels, palette);
}

uint8_t ost_display_transfer_byte(uint8_t dat)
{
  return spi_display_transfer(dat);
}

void ost_display_transfer_multi(uint8_t *buff, uint32_t btr)
{
  for (uint32_t i = 0; i < btr; i++)
  {
    buff[i] = spi_display_transfer(buff[i]);
  }
}

/*
 * Arduino Zero board initialization
 *
 * Good to know:
 *   - At reset, ResetHandler did the system clock configuration. Core is running at 48MHz.
 *   - Watchdog is disabled by default, unless someone plays with NVM User page
 *   - During reset, all PORT lines are configured as inputs with input buffers, output buffers and pull disabled.
 */
void arduino_mkrzero_init(void)
{
  // // Set Systick to 1ms interval, common to all Cortex-M variants
  // if ( SysTick_Config( SystemCoreClock / 1000 ) )
  // {
  //   // Capture error
  //   while ( 1 ) ;
  // }
  NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 2); /* set Priority for Systick Interrupt (2nd lowest) */

  // Clock PORT for Digital I/O
  PM->APBBMASK.reg |= PM_APBBMASK_PORT;
  //
  //  // Clock EIC for I/O interrupts
  PM->APBAMASK.reg |= PM_APBAMASK_EIC;

  // Clock SERCOM for Serial
  PM->APBCMASK.reg |= PM_APBCMASK_SERCOM0 | PM_APBCMASK_SERCOM1 | PM_APBCMASK_SERCOM2 | PM_APBCMASK_SERCOM3 | PM_APBCMASK_SERCOM4 | PM_APBCMASK_SERCOM5;

  // Clock TC/TCC for Pulse and Analog
  PM->APBCMASK.reg |= PM_APBCMASK_TCC0 | PM_APBCMASK_TCC1 | PM_APBCMASK_TCC2 | PM_APBCMASK_TC3 | PM_APBCMASK_TC4 | PM_APBCMASK_TC5;

  // Clock ADC/DAC for Analog
  PM->APBCMASK.reg |= PM_APBCMASK_ADC | PM_APBCMASK_DAC;

// Defining VERY_LOW_POWER breaks Arduino APIs since all pins are considered INPUT at startup
// However, it really lowers the power consumption by a factor of 20 in low power mode (0.03mA vs 0.6mA)
#ifdef VERY_LOW_POWER
  // Setup all pins (digital and analog) in INPUT mode (default is nothing)
  for (uint32_t ul = 0; ul < NUM_DIGITAL_PINS; ul++)
  {
    pinMode(ul, INPUT);
  }
#endif

  // Initialize Analog Controller
  // Setting clock
  while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)
    ;

  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCM_ADC) | // Generic Clock ADC
                      GCLK_CLKCTRL_GEN_GCLK0 |   // Generic Clock Generator 0 is source
                      GCLK_CLKCTRL_CLKEN;

  while (ADC->STATUS.bit.SYNCBUSY == 1)
    ; // Wait for synchronization of registers between the clock domains

  ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV512 | // Divide Clock by 512.
                   ADC_CTRLB_RESSEL_10BIT;      // 10 bits resolution as default

  ADC->SAMPCTRL.reg = 0x3f; // Set max Sampling Time Length

  while (ADC->STATUS.bit.SYNCBUSY == 1)
    ; // Wait for synchronization of registers between the clock domains

  ADC->INPUTCTRL.reg = ADC_INPUTCTRL_MUXNEG_GND; // No Negative input (Internal Ground)

  // Averaging (see datasheet table in AVGCTRL register description)
  ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_1 |  // 1 sample only (no oversampling nor averaging)
                     ADC_AVGCTRL_ADJRES(0x0ul); // Adjusting result by 0

  // analogReference( AR_DEFAULT ) ; // Analog Reference is AREF pin (3.3v)

  // Initialize DAC
  // Setting clock
  while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)
    ;
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCM_DAC) | // Generic Clock ADC
                      GCLK_CLKCTRL_GEN_GCLK0 |   // Generic Clock Generator 0 is source
                      GCLK_CLKCTRL_CLKEN;

  while (DAC->STATUS.bit.SYNCBUSY == 1)
    ;                                      // Wait for synchronization of registers between the clock domains
  DAC->CTRLB.reg = DAC_CTRLB_REFSEL_AVCC | // Using the 3.3V reference
                   DAC_CTRLB_EOEN;         // External Output Enable (Vout)
}
