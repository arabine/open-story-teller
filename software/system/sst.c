
/*--------------------------------------------------------------------------
           IMPORTATION DES FICHIERS DE DEFINITION DE LIBRAIRIE
--------------------------------------------------------------------------*/
#include "sst_conf.h"
#include <stdint.h>

/*--------------------------------------------------------------------------
                     PROTOTYPE DES FONCTIONS INTERNES
--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------
                              VARIABLES GLOBALES
--------------------------------------------------------------------------*/
/* Public-scope objects ----------------------------------------------------*/
uint16_t SST_currPrio_ = (uint16_t)0xFF;              /* current SST priority */
uint16_t SST_readySet_ = (uint16_t)0;                        /* SST ready-set */

typedef struct {
    SSTTask task__;
    SSTEvent *queue__;
    uint16_t end__;
    uint16_t head__;
    uint16_t tail__;
    uint16_t nUsed__;
    uint16_t mask__;
} TaskCB;

/* Local-scope objects -----------------------------------------------------*/
static TaskCB l_taskCB[SST_MAX_PRIO];


/*==========================================================================
======================  I N T E R R U P T I O N S  =========================
==========================================================================*/

/*==========================================================================
==========================  F O N C T I O N S  =============================
==========================================================================*/

/*..........................................................................*/
void SST_Task(SSTTask task, uint16_t prio, SSTEvent *queue, uint16_t qlen,
              SSTSignal sig, SSTParam par)
{
    SSTEvent ie;                                    /* initialization event */
    TaskCB *tcb  = &l_taskCB[prio - 1];
    tcb->task__  = task;
    tcb->queue__ = queue;
    tcb->end__   = qlen;
    tcb->head__  = (uint16_t)0;
    tcb->tail__  = (uint16_t)0;
    tcb->nUsed__ = (uint16_t)0;
    tcb->mask__  = (1 << (prio - 1));
    ie.sig = sig;
    ie.par = par;
    tcb->task__(ie);                                 /* initialize the task */
}
/*..........................................................................*/
void SST_Run(void) {
    SST_Start();                                              /* start ISRs */

    SST_INT_LOCK();
    SST_currPrio_ = (uint16_t)0;   /* set the priority for the SST idle loop */
    SST_Schedule_();                  /* process all events produced so far */
    SST_INT_UNLOCK();

    for (;;) {                                         /* the SST idle loop */
        SST_OnIdle();                        /* invoke the on-idle callback */
    }
}
/*..........................................................................*/
uint16_t SST_Post(uint16_t prio, SSTSignal sig, SSTParam par) {
    TaskCB *tcb = &l_taskCB[prio - 1];
    SST_INT_LOCK();
    if (tcb->nUsed__ < tcb->end__) {
        tcb->queue__[tcb->head__].sig = sig;/* insert the event at the head */
        tcb->queue__[tcb->head__].par = par;
        if ((++tcb->head__) == tcb->end__) {
            tcb->head__ = (uint16_t)0;                      /* wrap the head */
        }
        if ((++tcb->nUsed__) == (uint16_t)1) {           /* the first event? */
            SST_readySet_ |= tcb->mask__;   /* insert task to the ready set */
            SST_Schedule_();            /* check for synchronous preemption */
        }
        SST_INT_UNLOCK();
        return (uint16_t)1;                     /* event successfully posted */
    }
    else {
        SST_INT_UNLOCK();
        return (uint16_t)0;              /* queue full, event posting failed */
    }
}
/*..........................................................................*/
uint16_t SST_MutexLock(uint16_t prioCeiling) {
    uint16_t p;
    SST_INT_LOCK();
    p = SST_currPrio_;               /* the original SST priority to return */
    if (prioCeiling > SST_currPrio_) {
        SST_currPrio_ = prioCeiling;              /* raise the SST priority */
    }
    SST_INT_UNLOCK();
    return p;
}
/*..........................................................................*/
void SST_MutexUnlock(uint16_t orgPrio) {
    SST_INT_LOCK();
    if (orgPrio < SST_currPrio_) {
        SST_currPrio_ = orgPrio;    /* restore the saved priority to unlock */
        SST_Schedule_();    /* invoke scheduler after lowering the priority */
    }
    SST_INT_UNLOCK();
}
/*..........................................................................*/
/* NOTE: the SST scheduler is entered and exited with interrupts LOCKED */
void SST_Schedule_(void) {
   uint16_t pin = SST_currPrio_;               /* save the initial priority */
   uint16_t p;                                          /* the new priority */

    static const uint8_t log2Lkup[] = {
        0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
    };
      

    while ((p = log2Lkup[SST_readySet_]) > pin) {

   /* is the new priority higher than the initial? */
//   for( p=SST_MAX_PRIO; p>pin; p--) {
   //   if( SST_readySet_ & (1<<(p-1)) ) {
  //       if( p > pin ) {
      
            // reprise de l'ancien code
            TaskCB *tcb  = &l_taskCB[p - 1];
                                             /* get the event out of the queue */
            SSTEvent e = tcb->queue__[tcb->tail__];
            if ((++tcb->tail__) == tcb->end__) {
               tcb->tail__ = (uint16_t)0;
            }
            if ((--tcb->nUsed__) == (uint16_t)0) {/* is the queue becoming empty?*/
               SST_readySet_ &= ~tcb->mask__;     /* remove from the ready set */
            }
            SST_currPrio_ = p;        /* this becomes the current task priority */
            SST_INT_UNLOCK();                          /* unlock the interrupts */

            (*tcb->task__)(e);                             /* call the SST task */

            SST_INT_LOCK();            /* lock the interrupts for the next pass */
   //      }
   //   }
   }
   SST_currPrio_ = pin;                    /* restore the initial priority */
}

/*--------------------------------------------------------------------------
                             FIN FICHIER
--------------------------------------------------------------------------*/
/** @} */

