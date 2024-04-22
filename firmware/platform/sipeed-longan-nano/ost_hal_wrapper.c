#include "gd32vf103.h"
#include "systick.h"
#include <stdio.h>
#include <stdbool.h>
#include <ff.h>
#include "diskio.h"
#include "audio.h"
#include <string.h>
#include "spi0.h"
#include "spi1.h"
#include "ili9341.h"
#include "libutil.h"

// filename of wave file to play
const char filename[] = "out.wav"; // castemere_mono.wav";

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 2
//
// Set DISABLE_CHIP_SELECT to disable a second SPI device.
// For example, with the Ethernet shield, set DISABLE_CHIP_SELECT
// to 10 to disable the Ethernet controller.
const int8_t DISABLE_CHIP_SELECT = -1;

/* BUILTIN LED RED COLOR OF LONGAN BOARDS IS PIN PC13 */
// #define LED_PIN GPIO_PIN_13
// #define LED_GPIO_PORT GPIOC
// #define LED_GPIO_CLK RCU_GPIOC

/* BUILTIN LED GREEN*/
#define LED_PIN BIT(1)
#define LED_GPIO_PORT GPIOA
#define LED_GPIO_CLK RCU_GPIOA

// ----------------------------------------------------------------------------
// SYSTEM VARIABLES
// ----------------------------------------------------------------------------
static volatile uint32_t msTicks = 0;
static volatile bool tick_1s = false;
static volatile uint32_t tick_1s_counter = 0;

static volatile bool tick_10ms = false;
static volatile uint32_t tick_10ms_counter = 0;

void longan_led_on()
{
    GPIO_BC(LED_GPIO_PORT) = LED_PIN;
}

void longan_led_off()
{
    GPIO_BOP(LED_GPIO_PORT) = LED_PIN;
}

void longan_led_init()
{
    /* enable the led clock */
    rcu_periph_clock_enable(LED_GPIO_CLK);
    /* configure led GPIO port */
    gpio_init(LED_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LED_PIN);

    longan_led_off();
}

void longan_bp1_init()
{
    /* enable the KEY_B clock */
    rcu_periph_clock_enable(RCU_GPIOB);

    /* configure button pin as input */
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
}

static void init_uart0(void)
{
    // enable GPIO clock
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

    // enable USART0 clock
    rcu_periph_clock_enable(RCU_USART0);
    // configure USART0
    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200U);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);
}

static void system_tick_handler()
{
    msTicks++; /* See startup file startup_gd32vf103.S for SysTick vector */
    tick_1s_counter++;
    if (tick_1s_counter >= 1000)
    {
        tick_1s_counter = 0;
        tick_1s = true;
    }

    tick_10ms_counter++;
    if (tick_10ms_counter >= 10)
    {
        tick_10ms_counter = 0;
        tick_10ms = true;
    }

    disk_timerproc();
}

void TIMER0_UP_IRQHandler(void)
{
    timer_interrupt_flag_clear(TIMER0, TIMER_INT_FLAG_UP);
    system_tick_handler();
}

void timer0_irq_init(void)
{
    timer_parameter_struct tpa;

    rcu_periph_clock_enable(RCU_TIMER0);
    timer_struct_para_init(&tpa);
    tpa.prescaler = 1080 - 1;      // prescaler (108MHz -> 100KHz)
    tpa.period = 100 - 1;          // max value of counting up (100KHz -> 1000Hz = 1ms)
    tpa.repetitioncounter = 1 - 1; // the num of overflows that issues update IRQ. (1ms*10 = 10ms)
    timer_init(TIMER0, &tpa);
    timer_interrupt_enable(TIMER0, TIMER_INT_UP);
    timer_enable(TIMER0);

    eclic_enable_interrupt(TIMER0_UP_IRQn);
}

void timer0_irq_stop(void)
{
    timer_interrupt_disable(TIMER0, TIMER_INT_UP);
    timer_disable(TIMER0);
}

#define CONFIG_TICKS (TIMER_FREQ / 1000)

void eclic_mtip_handler(void)
{                                 /* SysTick interrupt Handler. */
    SysTick_Reload(CONFIG_TICKS); /* Call SysTick_Reload to reload timer. */
}

// ----------------------------------------------------------------------------
// SYSTEM HAL
// ----------------------------------------------------------------------------
void ost_system_initialize()
{
    SystemInit();

    // ECLIC init
    eclic_init(ECLIC_NUM_INTERRUPTS);
    eclic_mode_enable();

    // printf("After ECLIC mode enabled, the mtvec value is %x \n\n\r", read_csr(mtvec));

    // // It must be NOTED:
    //  //    * In the RISC-V arch, if user mode and PMP supported, then by default if PMP is not configured
    //  //      with valid entries, then user mode cannot access any memory, and cannot execute any instructions.
    //  //    * So if switch to user-mode and still want to continue, then you must configure PMP first
    // pmp_open_all_space();
    // switch_m2u_mode();

    /* Before enter into main, add the cycle/instret disable by default to save power,
    only use them when needed to measure the cycle/instret */
    disable_mcycle_minstret();

    longan_led_init();
    init_uart0();
    longan_bp1_init();

    (void)SysTick_Config(CONFIG_TICKS);

    timer0_irq_init(); // for TIMER0_UP_IRQHandler

    uint32_t bp1_counter = 0;
    uint32_t led_counter = 0;

    bool bp1_is_on = false;
    bool led_is_on = false;
    bool led_running = false;

    // Enable the interrupts. From now on interrupt handlers can be executed
    eclic_global_interrupt_enable();
}

void system_putc(char ch)
{
    usart_data_transmit(USART0, (uint8_t)ch);
    while (usart_flag_get(USART0, USART_FLAG_TBE) == RESET)
    {
    }
    //  return ch;
}

void ost_system_delay_ms(uint32_t delay)
{
    delay_1ms(delay);
    /*
    uint32_t curTicks;

    curTicks = msTicks;
    while ((msTicks - curTicks) < delay) ;*/
}

// ----------------------------------------------------------------------------
// SDCARD HAL
// ----------------------------------------------------------------------------
void spi_initialize(uint8_t id)
{
    spi1_initialize(0);
}

void sdcard_set_slow_clock()
{
    spi1_set_fclk_slow();
}

void sdcard_set_fast_clock()
{
    spi1_set_fclk_fast();
}

void ost_hal_sdcard_cs_high()
{
    void spi1_cs_high();
}

void ost_hal_sdcard_cs_low()
{
    void spi1_cs_low();
}

uint8_t ost_hal_sdcard_spi_transfer(uint8_t dat)
{
    return xchg_spi1(dat);
}

void sdcard_spi_recv_multi(uint8_t *buff, uint32_t btr)
{
    for (uint32_t i = 0; i < btr; i++)
    {
        buff[i] = xchg_spi1(0xff);
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
    // Init DC GPIO
    rcu_periph_clock_enable(DC_GPIO_CLK);

    // D/C (PA1) GPIO pin configuration
    gpio_init(DC_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, DC_PIN);

    spi0_initialize(0);
    spi0_set_fclk_fast();
    ili9341_read_id();
    ili9341_initialize();
    ili9341_set_rotation(1);

    ili9341_fill();
}

void ost_display_dc_high()
{
    GPIO_BOP(DC_GPIO_PORT) = DC_PIN;
}

void ost_display_dc_low()
{
    GPIO_BC(DC_GPIO_PORT) = DC_PIN;
}

void ost_display_draw_h_line(uint16_t y, uint8_t *pixels, uint8_t *palette)
{
    ili9341_draw_h_line(y, pixels, palette);
}
