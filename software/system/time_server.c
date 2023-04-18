#include <time.h>
#include "time_server.h"

typedef struct
{
    uint8_t Id;
    uint16_t Counter;         //! Expiring timer value in ticks from TimerContext
    uint16_t Period;       //! Reload Value when Timer is restarted
    bool IsRunning;             //! Is the timer currently running
    void ( *Callback )( void ); //! Timer IRQ callback function
} TimerEvent_t;


/*!
 * safely execute call back
 */
#define exec_cb( _callback_ )     \
  do {                          \
      if( _callback_ == NULL )    \
      {                           \
        while(1);                 \
      }                           \
      else                        \
      {                           \
        _callback_( );               \
      }                           \
  } while(0);                   


/*!
 * Timers list head pointer
 */
static TimerEvent_t gList[NUMBER_OF_TIMERS]; // FIXME: 


uint8_t TimerInit(uint16_t period, void ( *cb )( void ))
{
    uint8_t id = 0U;
    
    for (uint8_t i = 0U; i < NUMBER_OF_TIMERS; i++)
    {
        if (gList[i].Id == 0U) // free slot
        {
            id = (i+1);
            gList[i].Id = id; // take it
            gList[i].Period = period;
            gList[i].Callback = cb;
            gList[i].IsRunning = false;
            gList[i].Counter = 0U;
            break;
        }
    }
    
    return id;
}


void TimerStart( uint8_t id )
{
    for (uint8_t i = 0U; i < NUMBER_OF_TIMERS; i++)
    {
        if ((gList[i].Id == id) && (gList[i].Callback != NULL) && (gList[i].Period != 0))
        {
            gList[i].Counter = 0U;
            gList[i].IsRunning = true;
            break;
        }
    }
}

void TimerStop( uint8_t id )
{
    for (uint8_t i = 0U; i < NUMBER_OF_TIMERS; i++)
    {
        if (gList[i].Id == id)
        {
            gList[i].IsRunning = false;
            break;
        }
    }
}


void TimeServer_Initialize( void )
{
    for (uint8_t i = 0U; i < NUMBER_OF_TIMERS; i++)
    {
        gList[i].Callback = NULL;
        gList[i].IsRunning = false;
        gList[i].Counter = 0U;
        gList[i].Period = 0U;
    }
}

void TimeServer_IrqHandler( void )
{
    for (uint8_t i = 0U; i < NUMBER_OF_TIMERS; i++)
    {
        if (gList[i].IsRunning)
        {
            gList[i].Counter++;
            if (gList[i].Counter >= gList[i].Period)
            {
                gList[i].IsRunning = false;
                exec_cb(gList[i].Callback);
            }
        }
    }
    
}
