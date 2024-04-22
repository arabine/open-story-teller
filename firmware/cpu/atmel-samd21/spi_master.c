/*
 * Copyright (c) 2016, Alex Taradov <alex@taradov.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*- Includes ----------------------------------------------------------------*/
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "samd21.h"
#include "hal_gpio.h"
#include "spi_master.h"
#include "arduino_platform.h"
#include "ost_hal.h"

/*- Definitions -------------------------------------------------------------*/
/*
 +------------+------------------+--------+-----------------+--------+-----------------------+---------+---------+--------+--------+----------+----------+
 | Pin number |  MKR  Board pin  |  PIN   | Notes           | Peri.A |     Peripheral B      | Perip.C | Perip.D | Peri.E | Peri.F | Periph.G | Periph.H |
 |            |                  |        |                 |   EIC  | ADC |  AC | PTC | DAC | SERCOMx | SERCOMx |  TCCx  |  TCCx  |    COM   | AC/GLCK  |
 |            |                  |        |                 |(EXTINT)|(AIN)|(AIN)|     |     | (x/PAD) | (x/PAD) | (x/WO) | (x/WO) |          |          |
 +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
 |            | SD SPI           |        |                 |        |     |     |     |     |         |         |        |        |          |          |
 | 26         |                  |  PA12  | SD MOSI         |   12   |     |     |     |     |  *2/00  |   4/00  | TCC2/0 | TCC0/6 |          | AC/CMP0  |
 | 27         |                  |  PA13  | SD SCK          |   13   |     |     |     |     |  *2/01  |   4/01  | TCC2/1 | TCC0/7 |          | AC/CMP1  |
 | 28         |                  |  PA14  | SD SS           |   14   |     |     |     |     |   2/02  |   4/02  |  TC3/0 | TCC0/4 |          | GCLK_IO0 |
 | 29         |                  |  PA15  | SD MISO         |   15   |     |     |     |     |  *2/03  |   4/03  |  TC3/1 | TCC0/5 |          | GCLK_IO1 |
 | 30         |                  |  PA27  | SD CD           |   15   |     |     |     |     |         |         |        |        |          | GCLK_IO0 |
*/

HAL_GPIO_PIN(MOSI,            A, 12);
HAL_GPIO_PIN(MISO,            A, 15);
HAL_GPIO_PIN(SCLK,            A, 13);
HAL_GPIO_PIN(SS,              A, 14);
// HAL_GPIO_PIN(CD,              A, 27);
#define SPI_SERCOM                  SERCOM4
#define SERCOMx_GCLK_ID_CORE    GCM_SERCOM4_CORE
#define PM_APBCMASK_SERCOMx     PM_APBCMASK_SERCOM4
 #define PAD_SPI1_TX   SPI_PAD_0_SCK_1
#define PAD_SPI1_RX   SERCOM_RX_PAD_3

//-----------------------------------------------------------------------------
void spi_init(int freq, int mode)
{
  int baud = F_CPU / (2 * freq) - 1;

  if (baud < 0)
    baud = 0;

  if (baud > 255)
    baud = 255;

  freq = F_CPU / (2 * (baud + 1));

  HAL_GPIO_MISO_in();
  HAL_GPIO_MISO_pmuxen(PIO_SERCOM_ALT);

  HAL_GPIO_MOSI_out();
  HAL_GPIO_MOSI_pmuxen(PIO_SERCOM_ALT);

  HAL_GPIO_SCLK_out();
  HAL_GPIO_SCLK_pmuxen(PIO_SERCOM_ALT);

  HAL_GPIO_SS_out();
  HAL_GPIO_SS_set();

  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(SERCOMx_GCLK_ID_CORE) | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0;
  while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

  SPI_SERCOM->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_SWRST;
  while (SPI_SERCOM->SPI.CTRLA.reg & SERCOM_SPI_CTRLA_SWRST);

  SPI_SERCOM->SPI.CTRLB.reg = SERCOM_SPI_CTRLB_RXEN;

  SPI_SERCOM->SPI.BAUD.reg = baud;

  SPI_SERCOM->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_ENABLE |
      SERCOM_SPI_CTRLA_DIPO(PAD_SPI1_RX) | SERCOM_SPI_CTRLA_DOPO(PAD_SPI1_TX) |
      ((mode & 1) ? SERCOM_SPI_CTRLA_CPHA : 0) |
      ((mode & 2) ? SERCOM_SPI_CTRLA_CPOL : 0) |
      SERCOM_SPI_CTRLA_MODE_SPI_MASTER;
}


//-----------------------------------------------------------------------------
void spi_ss(int state)
{
  HAL_GPIO_SS_write(state);
}

//-----------------------------------------------------------------------------
uint8_t spi_transfer(uint8_t byte)
{
//  while (!SPI_SERCOM->SPI.INTFLAG.bit.DRE);
  SPI_SERCOM->SPI.DATA.bit.DATA = byte;
  while (!SPI_SERCOM->SPI.INTFLAG.bit.RXC);
  return SPI_SERCOM->SPI.DATA.bit.DATA;  // Reading data
}

void spi_reset()
{
    //Setting the Software Reset bit to 1
    //Setting the Software Reset bit to 1
  SPI_SERCOM->SPI.CTRLA.bit.SWRST = 1;

  //Wait both bits Software Reset from CTRLA and SYNCBUSY are equal to 0
  while(SPI_SERCOM->SPI.CTRLA.bit.SWRST || SPI_SERCOM->SPI.SYNCBUSY.bit.SWRST);
}

void spi_enable()
{
  //Setting the enable bit to 1
  SPI_SERCOM->SPI.CTRLA.bit.ENABLE = 1;

  while(SPI_SERCOM->SPI.SYNCBUSY.bit.ENABLE)
  {
    //Waiting then enable bit from SYNCBUSY is equal to 0;
  }
}

