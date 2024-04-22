/*------------------------------------------------------/
/ Copyright (c) 2020, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include "i2s.h"

#include "gd32vf103_gpio.h"
#include "gd32vf103_spi.h"
#include "gd32vf103_dma.h"

dma_parameter_struct dma_i2s2;


dma_parameter_struct dma_param;

void init_dma_i2s2(int32_t* memory_addr, uint32_t trans_number)
{
      rcu_periph_clock_enable(RCU_DMA1);

    dma_struct_para_init(&dma_i2s2);
    dma_i2s2.periph_addr = (uint32_t) &SPI_DATA(SPI2);
    dma_i2s2.periph_width = DMA_PERIPHERAL_WIDTH_32BIT;
    dma_i2s2.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_i2s2.memory_addr = (uint32_t) memory_addr;
    dma_i2s2.memory_width = DMA_MEMORY_WIDTH_16BIT;
    dma_i2s2.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_i2s2.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_i2s2.number = trans_number;
    dma_i2s2.priority = DMA_PRIORITY_HIGH;
    dma_init(DMA1, DMA_CH1, &dma_i2s2);
}

void init_i2s2(void)
{
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_SPI2);

    RCU_CTL &= ~(RCU_CTL_PLL1EN | RCU_CTL_PLL2EN);
    rcu_predv1_config(RCU_PREDV1_DIV2);
    rcu_pll2_config(RCU_PLL2_MUL12);
    rcu_i2s2_clock_config(RCU_I2S2SRC_CKPLL2_MUL2);
    RCU_CTL |= (RCU_CTL_PLL1EN | RCU_CTL_PLL2EN);

    gpio_pin_remap_config(GPIO_SWJ_DISABLE_REMAP, ENABLE);
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3 | GPIO_PIN_5);    

    i2s_init(SPI2, I2S_MODE_MASTERTX, I2S_STD_PHILLIPS, I2S_CKPL_HIGH);
    i2s_psc_config(SPI2, I2S_AUDIOSAMPLE_44K, I2S_FRAMEFORMAT_DT24B_CH32B, I2S_MCKOUT_DISABLE);
    i2s_enable(SPI2);
}

/*

void init_dma_i2s2(uint16_t *memory_addr, uint32_t trans_number)
{
    rcu_periph_clock_enable(RCU_DMA1);

    dma_struct_para_init(&dma_i2s2);
    dma_i2s2.periph_addr = (uint32_t) &SPI_DATA(SPI2);
    dma_i2s2.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    dma_i2s2.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_i2s2.memory_addr = (uint32_t) memory_addr;
    dma_i2s2.memory_width = DMA_MEMORY_WIDTH_16BIT;
    dma_i2s2.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_i2s2.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_i2s2.number = trans_number;
    dma_i2s2.priority = DMA_PRIORITY_HIGH;
    dma_init(DMA1, DMA_CH1, &dma_i2s2);
}

void init_i2s2(void)
{
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_SPI2);

    // Provide i2s clock by PLL2
    RCU_CTL &= ~(RCU_CTL_PLL1EN | RCU_CTL_PLL2EN);
    rcu_predv1_config(RCU_PREDV1_DIV2);
    rcu_pll2_config(RCU_PLL2_MUL12);
    rcu_i2s2_clock_config(RCU_I2S2SRC_CKPLL2_MUL2);
    RCU_CTL |= (RCU_CTL_PLL1EN | RCU_CTL_PLL2EN);

    // i2s2 pins I2S_CK: PB3*, I2S_WS: PA15*, I2S_SD: PB5 (*: JTAG function must be disable)
    gpio_pin_remap_config(GPIO_SWJ_DISABLE_REMAP, ENABLE);
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3 | GPIO_PIN_5);    

    i2s_init(SPI2, I2S_MODE_MASTERTX, I2S_STD_MSB, I2S_CKPL_HIGH);
    i2s_psc_config(SPI2, I2S_AUDIOSAMPLE_44K, I2S_FRAMEFORMAT_DT16B_CH16B, I2S_MCKOUT_DISABLE);
    i2s_enable(SPI2);
}
*/