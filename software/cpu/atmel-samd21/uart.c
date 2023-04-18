
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "samd21.h"
#include "hal_gpio.h"
#include "arduino_platform.h"

HAL_GPIO_PIN(UART_TX,  B, 22)
HAL_GPIO_PIN(UART_RX,  B, 23)

//-----------------------------------------------------------------------------
void uart_init(uint32_t baud)
{
  uint64_t br = (uint64_t)65536 * (F_CPU - 16 * baud) / F_CPU;

  // Start the Software Reset and wait for it to finish
	SERCOM5->USART.CTRLA.bit.SWRST = 1 ;
	while ( SERCOM5->USART.CTRLA.bit.SWRST || SERCOM5->USART.SYNCBUSY.bit.SWRST )	;

  HAL_GPIO_UART_TX_out();
  HAL_GPIO_UART_TX_pmuxen(PORT_PMUX_PMUXE_D);
  HAL_GPIO_UART_RX_in();
  HAL_GPIO_UART_RX_pmuxen(PORT_PMUX_PMUXE_D);

  PM->APBCMASK.reg |= PM_APBCMASK_SERCOM5;

  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(SERCOM5_GCLK_ID_CORE) |
      GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(0);

  while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY ) ; // Wait for synchronization

  SERCOM5->USART.CTRLA.reg =
      SERCOM_USART_CTRLA_DORD | SERCOM_USART_CTRLA_MODE_USART_INT_CLK |
      SERCOM_USART_CTRLA_RXPO(SERCOM_RX_PAD_3/*PAD3*/) | SERCOM_USART_CTRLA_TXPO(UART_TX_PAD_2/*PAD2*/);


  SERCOM5->USART.CTRLB.reg = SERCOM_USART_CTRLB_RXEN | SERCOM_USART_CTRLB_TXEN |
      SERCOM_USART_CTRLB_CHSIZE(0/*8 bits*/) |
      SERCOM_USART_CTRLA_SAMPR(0x0)			|	// 16x over sampling
								SERCOM_USART_CTRLA_RUNSTDBY			// run in standby mode
                ;

  SERCOM5->USART.BAUD.reg = (uint16_t)br;

  SERCOM5->USART.CTRLA.reg |= SERCOM_USART_CTRLA_ENABLE;
  SERCOM5->USART.CTRLA.bit.MODE = 0x1;    // USART internal clock
  SERCOM5->USART.CTRLA.bit.ENABLE = 1;

  while (SERCOM5->USART.SYNCBUSY.reg & SERCOM_USART_SYNCBUSY_ENABLE);
}

//-----------------------------------------------------------------------------
void uart_putc(char c)
{
  while (!(SERCOM5->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_DRE));
  SERCOM5->USART.DATA.reg = c;
}

