
#include "gd32vf103.h"
#include "spi0.h"

#define CS_PIN BIT(4)
#define CS_GPIO_PORT GPIOA
#define CS_GPIO_CLK RCU_GPIOA

void spi0_set_fclk_slow()
{
    SPI_CTL0(SPI0) = (SPI_CTL0(SPI0) & ~0x38) | 0x38;
}

void spi0_set_fclk_fast()
{
    SPI_CTL0(SPI0) = (SPI_CTL0(SPI0) & ~0x38) | 0x00;
}

void spi0_cs_high()
{
    GPIO_BOP(CS_GPIO_PORT) = CS_PIN;
}

void spi0_cs_low()
{
    GPIO_BC(CS_GPIO_PORT) = CS_PIN;
}


void spi0_initialize()
{
	spi_parameter_struct spi_init_struct;

    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_SPI0);

    /* SPI0_SCK(PA5) and SPI0_MOSI(PA7) GPIO pin configuration  */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5 | GPIO_PIN_7);
    /* SPI0_MISO(PA6) GPIO pin configuration  */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_6); // MISO
    /* SPI0_CS(PA4) GPIO pin configuration */
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);

    /* chip _select invalid*/
    spi0_cs_high();

    /* SPI0 parameter config */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_32; // SPI_PSC_32; (PCLK1=54MHz)
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI0, &spi_init_struct);

    /* set crc polynomial */
    // spi_crc_polynomial_set(SPI0, 7);
    /* enable SPI1 */
    spi_enable(SPI0);
}


/* Exchange a byte */
uint8_t xchg_spi0 (
	uint8_t dat	/* Data to send */
)
{
	while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE));
    spi_i2s_data_transmit(SPI0, dat);
    while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE));
    return(spi_i2s_data_receive(SPI0));     /* Return received byte */
}
