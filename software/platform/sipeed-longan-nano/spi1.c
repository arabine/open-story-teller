
#include "gd32vf103.h"
#include "spi1.h"

#define CS_PIN BIT(12)
#define CS_GPIO_PORT GPIOB
#define CS_GPIO_CLK RCU_GPIOB


/*
 PB15 SPI1_MOSI
 PB14 SPI1_MISO
 PB13 SPI1_SCLK
 PB12 SPI1_CS_TF
 */


void spi1_set_fclk_slow()
{
    SPI_CTL0(SPI1) = (SPI_CTL0(SPI1) & ~0x38) | 0x38;
}

void spi1_set_fclk_fast()
{
    SPI_CTL0(SPI1) = (SPI_CTL0(SPI1) & ~0x38) | 0x00;
}

void spi1_cs_high()
{
    GPIO_BOP(CS_GPIO_PORT) = CS_PIN;
}

void spi1_cs_low()
{
    GPIO_BC(CS_GPIO_PORT) = CS_PIN;
}


void spi1_initialize()
{
	spi_parameter_struct spi_init_struct;

    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_SPI1);

    /* SPI1_SCK(PB13), SPI1_MISO(PB14) and SPI1_MOSI(PB15) GPIO pin configuration */
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13 | GPIO_PIN_15);
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_14);
    /* SPI1_CS(PB12) GPIO pin configuration */
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);

    /* chip _select invalid*/
    spi1_cs_high();

    /* SPI1 parameter config */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_32; // SPI_PSC_32; (PCLK1=54MHz)
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI1, &spi_init_struct);

    /* set crc polynomial */
    spi_crc_polynomial_set(SPI1, 7);
    /* enable SPI1 */
    spi_enable(SPI1);
}


/* Exchange a byte */
uint8_t xchg_spi1 (
	uint8_t dat	/* Data to send */
)
{
	while(RESET == spi_i2s_flag_get(SPI1, SPI_FLAG_TBE));
    spi_i2s_data_transmit(SPI1, dat);
    while(RESET == spi_i2s_flag_get(SPI1, SPI_FLAG_RBNE));
    return(spi_i2s_data_receive(SPI1));     /* Return received byte */
}
