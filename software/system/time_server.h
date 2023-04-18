#ifndef __TIMESERVER_H__
#define __TIMESERVER_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

#define NUMBER_OF_TIMERS 30
     

uint8_t TimerInit(uint16_t period, void ( *cb )( void ));
void TimerStart( uint8_t id );
void TimerStop( uint8_t id );

void TimeServer_Initialize( void );
void TimeServer_IrqHandler( void );


#ifdef __cplusplus
}
#endif

#endif /* __TIMESERVER_H__*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
