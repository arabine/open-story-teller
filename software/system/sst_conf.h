/**
 * File: sst_conf.h
 */
#ifndef _SST_CONF_h
#define _SST_CONF_h

#include "global_defines.h"
#include "sst.h" 

/*--------------------------------------------------------------------------
                               DEFINITIONS
--------------------------------------------------------------------------*/
/* SST interrupt locking/unlocking */
#define SST_INT_LOCK()   {_DISI = 1;} 
#define SST_INT_UNLOCK() { _GIE = 1; }

/* maximum SST task priority */
#define SST_MAX_PRIO     8

/* the events used in the application */
enum Events {
    INIT_SIG,  /* initialization event */
	REINIT_SIG,
    RUN_SIG    /* normal run */
};

/* les priorités vont dans l'ordre croissant d'importance */
enum SSTPriorities {     /* the SST priorities don't need to be consecutive */
   TASK_IDLE = 0,
   /* task priorities from low to high */
   ISR_TIMER2_PRIO,
   ISR_TIMER3_PRIO,
   ISR_UART1_PRIO,
   ISR_UART2_PRIO,
   
   TASK_ESP32,
   TASK_TCP_IP,
   TASK_APP_SERENITY,
};

/*--------------------------------------------------------------------------
                          FONCTIONS EXPORTEES
--------------------------------------------------------------------------*/
void TASK_EmergencyStop(void);
void TASK_TopSecond(void);
void TASK_Initialize(void);

#endif /* _SST_CONF_h */

/*--------------------------------------------------------------------------
                             FIN FICHIER
--------------------------------------------------------------------------*/
