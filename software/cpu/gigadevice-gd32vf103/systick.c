/*!
    \file  systick.c
    \brief the systick configuration file

    \version 2019-6-5, V1.0.0, firmware for GD32VF103
*/

/*
    Copyright (c) 2019, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "gd32vf103.h"
#include "systick.h"

/*!
    \brief      delay a time in milliseconds
    \param[in]  count: count in milliseconds
    \param[out] none
    \retval     none
*/
void delay_1ms(uint32_t count)
{
    uint64_t start_mtime, delta_mtime;

    // Don't start measuruing until we see an mtime tick
    uint64_t tmp = get_timer_value();
    do {
    start_mtime = get_timer_value();
    } while (start_mtime == tmp);

    do {
    delta_mtime = get_timer_value() - start_mtime;
    }while(delta_mtime <(SystemCoreClock/4000.0 *count ));
}

typedef struct {
    volatile uint64_t MTIMER;                  /*!< Offset: 0x000 (R/W)  System Timer current value 64bits Register */
    volatile uint64_t MTIMERCMP;               /*!< Offset: 0x008 (R/W)  System Timer compare Value 64bits Register */
    volatile uint32_t RESERVED0[0x3F8];        /*!< Offset: 0x010 - 0xFEC Reserved */
    volatile uint32_t MSFTRST;                 /*!< Offset: 0xFF0 (R/W)  System Timer Software Core Reset Register */
    volatile uint32_t RESERVED1;               /*!< Offset: 0xFF4 Reserved */
    volatile uint32_t MTIMECTL;                /*!< Offset: 0xFF8 (R/W)  System Timer Control Register, previously MSTOP register */
    volatile uint32_t MSIP;                    /*!< Offset: 0xFFC (R/W)  System Timer SW interrupt Register */
} SysTimer_Type;

#define SysTimer_MTIMECTL_TIMESTOP_Pos      0U                                          /*!< SysTick Timer MTIMECTL: TIMESTOP bit Position */
#define SysTimer_MTIMECTL_TIMESTOP_Msk      (1UL << SysTimer_MTIMECTL_TIMESTOP_Pos)     /*!< SysTick Timer MTIMECTL: TIMESTOP Mask */
#define SysTimer_MTIMECTL_CMPCLREN_Pos      1U                                          /*!< SysTick Timer MTIMECTL: CMPCLREN bit Position */
#define SysTimer_MTIMECTL_CMPCLREN_Msk      (1UL << SysTimer_MTIMECTL_CMPCLREN_Pos)     /*!< SysTick Timer MTIMECTL: CMPCLREN Mask */
#define SysTimer_MTIMECTL_CLKSRC_Pos        2U                                          /*!< SysTick Timer MTIMECTL: CLKSRC bit Position */
#define SysTimer_MTIMECTL_CLKSRC_Msk        (1UL << SysTimer_MTIMECTL_CLKSRC_Pos)       /*!< SysTick Timer MTIMECTL: CLKSRC Mask */

#define SysTimer_MSIP_MSIP_Pos              0U                                          /*!< SysTick Timer MSIP: MSIP bit Position */
#define SysTimer_MSIP_MSIP_Msk              (1UL << SysTimer_MSIP_MSIP_Pos)             /*!< SysTick Timer MSIP: MSIP Mask */

#define SysTimer_MTIMER_Msk                 (0xFFFFFFFFFFFFFFFFULL)                     /*!< SysTick Timer MTIMER value Mask */
#define SysTimer_MTIMERCMP_Msk              (0xFFFFFFFFFFFFFFFFULL)                     /*!< SysTick Timer MTIMERCMP value Mask */
#define SysTimer_MTIMECTL_Msk               (0xFFFFFFFFUL)                              /*!< SysTick Timer MTIMECTL/MSTOP value Mask */
#define SysTimer_MSIP_Msk                   (0xFFFFFFFFUL)                              /*!< SysTick Timer MSIP   value Mask */
#define SysTimer_MSFTRST_Msk                (0xFFFFFFFFUL)                              /*!< SysTick Timer MSFTRST value Mask */

#define SysTimer_MSFRST_KEY                 (0x80000A5FUL)                              /*!< SysTick Timer Software Reset Request Key */


/* System Timer Memory mapping of Device  */
#define SysTimer_BASE                       TIMER_CTRL_ADDR                         /*!< SysTick Base Address */
#define SysTimer                            ((SysTimer_Type *) SysTimer_BASE)           /*!< SysTick configuration struct */


/* ##################################    SysTimer function  ############################################ */
/**
 * \defgroup NMSIS_Core_SysTimer SysTimer Functions
 * \brief    Functions that configure the Core System Timer.
 * @{
 */
/**
 * \brief  Set system timer load value
 * \details
 * This function set the system timer load value in MTIMER register.
 * \param [in]  value   value to set system timer MTIMER register.
 * \remarks
 * - Load value is 64bits wide.
 * - \ref SysTimer_GetLoadValue
 */
void SysTimer_SetLoadValue(uint64_t value)
{
    SysTimer->MTIMER = value;
}

/**
 * \brief  Get system timer load value
 * \details
 * This function get the system timer current value in MTIMER register.
 * \return  current value(64bit) of system timer MTIMER register.
 * \remarks
 * - Load value is 64bits wide.
 * - \ref SysTimer_SetLoadValue
 */
uint64_t SysTimer_GetLoadValue(void)
{
    return SysTimer->MTIMER;
}

/**
 * \brief  Set system timer compare value
 * \details
 * This function set the system Timer compare value in MTIMERCMP register.
 * \param [in]  value   compare value to set system timer MTIMERCMP register.
 * \remarks
 * - Compare value is 64bits wide.
 * - If compare value is larger than current value timer interrupt generate.
 * - Modify the load value or compare value less to clear the interrupt.
 * - \ref SysTimer_GetCompareValue
 */
void SysTimer_SetCompareValue(uint64_t value)
{
    SysTimer->MTIMERCMP = value;
}

/**
 * \brief  Get system timer compare value
 * \details
 * This function get the system timer compare value in MTIMERCMP register.
 * \return  compare value of system timer MTIMERCMP register.
 * \remarks
 * - Compare value is 64bits wide.
 * - \ref SysTimer_SetCompareValue
 */
uint64_t SysTimer_GetCompareValue(void)
{
    return SysTimer->MTIMERCMP;
}

/**
 * \brief  Enable system timer counter running
 * \details
 * Enable system timer counter running by clear
 * TIMESTOP bit in MTIMECTL register.
 */
void SysTimer_Start(void)
{
    SysTimer->MTIMECTL &= ~(SysTimer_MTIMECTL_TIMESTOP_Msk);
}

/**
 * \brief  Stop system timer counter running
 * \details
 * Stop system timer counter running by set
 * TIMESTOP bit in MTIMECTL register.
 */
void SysTimer_Stop(void)
{
    SysTimer->MTIMECTL |= SysTimer_MTIMECTL_TIMESTOP_Msk;
}

/**
 * \brief  Set system timer control value
 * \details
 * This function set the system timer MTIMECTL register value.
 * \param [in]  mctl    value to set MTIMECTL register
 * \remarks
 * - Bit TIMESTOP is used to start and stop timer.
 *   Clear TIMESTOP bit to 0 to start timer, otherwise to stop timer.
 * - Bit CMPCLREN is used to enable auto MTIMER clear to zero when MTIMER >= MTIMERCMP.
 *   Clear CMPCLREN bit to 0 to stop auto clear MTIMER feature, otherwise to enable it.
 * - Bit CLKSRC is used to select timer clock source.
 *   Clear CLKSRC bit to 0 to use *mtime_toggle_a*, otherwise use *core_clk_aon*
 * - \ref SysTimer_GetControlValue
 */
void SysTimer_SetControlValue(uint32_t mctl)
{
    SysTimer->MTIMECTL = (mctl & SysTimer_MTIMECTL_Msk);
}

/**
 * \brief  Get system timer control value
 * \details
 * This function get the system timer MTIMECTL register value.
 * \return  MTIMECTL register value
 * \remarks
 * - \ref SysTimer_SetControlValue
 */
uint32_t SysTimer_GetControlValue(void)
{
    return (SysTimer->MTIMECTL & SysTimer_MTIMECTL_Msk);
}

/**
 * \brief  Trigger or set software interrupt via system timer
 * \details
 * This function set the system timer MSIP bit in MSIP register.
 * \remarks
 * - Set system timer MSIP bit and generate a SW interrupt.
 * - \ref SysTimer_ClearSWIRQ
 * - \ref SysTimer_GetMsipValue
 */
void SysTimer_SetSWIRQ(void)
{
    SysTimer->MSIP |= SysTimer_MSIP_MSIP_Msk;
}

/**
 * \brief  Clear system timer software interrupt pending request
 * \details
 * This function clear the system timer MSIP bit in MSIP register.
 * \remarks
 * - Clear system timer MSIP bit in MSIP register to clear the software interrupt pending.
 * - \ref SysTimer_SetSWIRQ
 * - \ref SysTimer_GetMsipValue
 */
void SysTimer_ClearSWIRQ(void)
{
    SysTimer->MSIP &= ~SysTimer_MSIP_MSIP_Msk;
}

/**
 * \brief  Get system timer MSIP register value
 * \details
 * This function get the system timer MSIP register value.
 * \return    Value of Timer MSIP register.
 * \remarks
 * - Bit0 is SW interrupt flag.
 *   Bit0 is 1 then SW interrupt set. Bit0 is 0 then SW interrupt clear.
 * - \ref SysTimer_SetSWIRQ
 * - \ref SysTimer_ClearSWIRQ
 */
uint32_t SysTimer_GetMsipValue(void)
{
    return (uint32_t)(SysTimer->MSIP & SysTimer_MSIP_Msk);
}

/**
 * \brief  Set system timer MSIP register value
 * \details
 * This function set the system timer MSIP register value.
 * \param [in]  msip   value to set MSIP register
 */
void SysTimer_SetMsipValue(uint32_t msip)
{
    SysTimer->MSIP = (msip & SysTimer_MSIP_Msk);
}

/**
 * \brief  Do software reset request
 * \details
 * This function will do software reset request through MTIMER
 * - Software need to write \ref SysTimer_MSFRST_KEY to generate software reset request
 * - The software request flag can be cleared by reset operation to clear
 * \remarks
 * - The software reset is sent to SoC, SoC need to generate reset signal and send back to Core
 * - This function will not return, it will do while(1) to wait the Core reset happened
 */
void SysTimer_SoftwareReset(void)
{
    SysTimer->MSFTRST = SysTimer_MSFRST_KEY;
    while(1);
}


/**
 * \brief   System Tick Configuration
 * \details Initializes the System Timer and its non-vector interrupt, and starts the System Tick Timer.
 *
 *  In our default implementation, the timer counter will be set to zero, and it will start a timer compare non-vector interrupt
 *  when it matchs the ticks user set, during the timer interrupt user should reload the system tick using \ref SysTick_Reload function
 *  or similar function written by user, so it can produce period timer interrupt.
 * \param [in]  ticks  Number of ticks between two interrupts.
 * \return          0  Function succeeded.
 * \return          1  Function failed.
 * \remarks
 * - For \ref __NUCLEI_N_REV >= 0x0104, the CMPCLREN bit in MTIMECTL is introduced,
 *   but we assume that the CMPCLREN bit is set to 0, so MTIMER register will not be
 *   auto cleared to 0 when MTIMER >= MTIMERCMP.
 * - When the variable \ref __Vendor_SysTickConfig is set to 1, then the
 *   function \ref SysTick_Config is not included.
 * - In this case, the file <b><Device>.h</b> must contain a vendor-specific implementation
 *   of this function.
 * - If user need this function to start a period timer interrupt, then in timer interrupt handler
 *   routine code, user should call \ref SysTick_Reload with ticks to reload the timer.
 * - This function only available when __SYSTIMER_PRESENT == 1 and __ECLIC_PRESENT == 1 and __Vendor_SysTickConfig == 0
 * \sa
 * - \ref SysTimer_SetCompareValue; SysTimer_SetLoadValue
 */
uint32_t SysTick_Config(uint64_t ticks)
{
    SysTimer_SetLoadValue(0);
    SysTimer_SetCompareValue(ticks);

    eclic_global_interrupt_enable();
    eclic_enable_interrupt(CLIC_INT_TMR);
    
    // ECLIC_SetShvIRQ(SysTimer_IRQn, ECLIC_NON_VECTOR_INTERRUPT);
    // ECLIC_SetLevelIRQ(SysTimer_IRQn, 0);
    // ECLIC_EnableIRQ(SysTimer_IRQn);
    return (0UL);
}

#ifndef   __USUALLY
  #define __USUALLY(exp)                         __builtin_expect((exp), 1)
#endif

/**
 * \brief   System Tick Reload
 * \details Reload the System Timer Tick when the MTIMECMP reached TIME value
 *
 * \param [in]  ticks  Number of ticks between two interrupts.
 * \return          0  Function succeeded.
 * \return          1  Function failed.
 * \remarks
 * - For \ref __NUCLEI_N_REV >= 0x0104, the CMPCLREN bit in MTIMECTL is introduced,
 *   but for this \ref SysTick_Config function, we assume this CMPCLREN bit is set to 0,
 *   so in interrupt handler function, user still need to set the MTIMERCMP or MTIMER to reload
 *   the system tick, if vendor want to use this timer's auto clear feature, they can define
 *   \ref __Vendor_SysTickConfig to 1, and implement \ref SysTick_Config and \ref SysTick_Reload functions.
 * - When the variable \ref __Vendor_SysTickConfig is set to 1, then the
 *   function \ref SysTick_Reload is not included.
 * - In this case, the file <b><Device>.h</b> must contain a vendor-specific implementation
 *   of this function.
 * - This function only available when __SYSTIMER_PRESENT == 1 and __ECLIC_PRESENT == 1 and __Vendor_SysTickConfig == 0
 * - Since the MTIMERCMP value might overflow, if overflowed, MTIMER will be set to 0, and MTIMERCMP set to ticks
 * \sa
 * - \ref SysTimer_SetCompareValue
 * - \ref SysTimer_SetLoadValue
 */
uint32_t SysTick_Reload(uint64_t ticks)
{
    uint64_t cur_ticks = SysTimer->MTIMER;
    uint64_t reload_ticks = ticks + cur_ticks;

    if (__USUALLY(reload_ticks > cur_ticks)) {
        SysTimer->MTIMERCMP = reload_ticks;
    } else {
        /* When added the ticks value, then the MTIMERCMP < TIMER,
         * which means the MTIMERCMP is overflowed,
         * so we need to reset the counter to zero */
        SysTimer->MTIMER = 0;
        SysTimer->MTIMERCMP = ticks;
    }

    return (0UL);
}

