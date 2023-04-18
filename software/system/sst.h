
#ifndef _SST_H
#define _SST_H

#include "global_defines.h"

/*--------------------------------------------------------------------------
                              DEFINITIONS
--------------------------------------------------------------------------*/
typedef uint16_t SSTSignal;
typedef uint16_t SSTParam;

typedef struct SSTEventTag SSTEvent;
struct SSTEventTag {
    SSTSignal sig;
    SSTParam  par;
};

typedef void (*SSTTask)(SSTEvent e);

/* SST interrupt entry and exit */
#define SST_ISR_ENTRY(pin_, isrPrio_) do { \
    (pin_) = SST_currPrio_; \
    SST_currPrio_ = (isrPrio_); \
    SST_INT_UNLOCK(); \
} while (0)

#define SST_ISR_EXIT(pin_) do { \
    SST_INT_LOCK(); \
    SST_currPrio_ = (pin_); \
    SST_Schedule_(); \
} while (0)

/*--------------------------------------------------------------------------
                          VARIABLES EXPORTEES
--------------------------------------------------------------------------*/
/* public-scope objects */
extern uint16_t SST_currPrio_;     /* current priority of the executing task */
extern uint16_t SST_readySet_;     /* SST ready-set */


/*--------------------------------------------------------------------------
                          FONCTIONS EXPORTEES
--------------------------------------------------------------------------*/
void SST_Init(void);
void SST_Task(SSTTask task, uint16_t prio, SSTEvent *queue, uint16_t qlen,
              SSTSignal sig, SSTParam  par);
void SST_Start(void);
void SST_Run(void);
void SST_OnIdle(void);
void SST_Exit(void);

uint16_t SST_Post(uint16_t prio, SSTSignal sig, SSTParam  par);

uint16_t SST_MutexLock(uint16_t prioCeiling);
void SST_MutexUnlock(uint16_t orgPrio);

void SST_Schedule_(void);

#endif /* _SST_H */

/*--------------------------------------------------------------------------
                             FIN FICHIER
--------------------------------------------------------------------------*/


