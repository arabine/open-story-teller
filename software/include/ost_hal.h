#ifndef OST_HAL_H
#define OST_HAL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define OST_ID_SPI_FOR_SDCARD 0

#define portDISABLE_INTERRUPTS() __asm volatile("csrc mstatus, 8")
#define portENABLE_INTERRUPTS() __asm volatile("csrs mstatus, 8")

#define portNOP() __asm volatile(" nop ")

    // ----------------------------------------------------------------------------
    // SHARED TYPES
    // ----------------------------------------------------------------------------
    typedef struct
    {
        uint16_t x;
        uint16_t y;
        uint16_t width;
        uint16_t height;
    } rect_t;

    typedef struct
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    } color_t;

    typedef enum
    {
        OST_GPIO_ROTARY_A,
        OST_GPIO_ROTARY_B,
        OST_GPIO_DEBUG_LED,
    } ost_hal_gpio_t;

    // ----------------------------------------------------------------------------
    // SYSTEM HAL
    // ----------------------------------------------------------------------------
    void ost_system_initialize();
    void system_putc(char ch);
    void ost_system_delay_ms(uint32_t delay);

    int ost_hal_gpio_get(ost_hal_gpio_t gpio);
    void ost_hal_gpio_set(ost_hal_gpio_t gpio, int value);

    // ----------------------------------------------------------------------------
    // SDCARD HAL
    // ----------------------------------------------------------------------------

    /**
     * @brief Deselect the SD-Card by driving the Chip Select to high level (eg: 3.3V)
     *
     */
    void ost_hal_sdcard_cs_high();

    /**
     * @brief Deselect the SD-Card by driving the Chip Select to low level (eg: 0V)
     *
     */
    void ost_hal_sdcard_cs_low();

    /**
     * @brief
     *
     * @param dat Data to send
     * @return uint8_t
     */
    uint8_t ost_hal_sdcard_spi_transfer(uint8_t dat);

    /**
     * @brief Return 1 if the SD card is physically inserted, otherwise 0
     *
     * @return uint8_t SD card is present or not
     */
    uint8_t ost_hal_sdcard_get_presence();

    // ----------------------------------------------------------------------------
    // DISPLAY HAL
    // ----------------------------------------------------------------------------
    void ost_display_initialize();
    void ost_display_draw_h_line(uint16_t y, uint8_t *pixels, uint8_t *palette);
    void ost_display_dc_high();
    void ost_display_dc_low();
    void ost_display_ss_high();
    void ost_display_ss_low();
    uint8_t ost_display_transfer_byte(uint8_t dat);
    void ost_display_transfer_multi(uint8_t *buff, uint32_t btr);

#ifdef __cplusplus
}
#endif

#endif // OST_HAL_H
