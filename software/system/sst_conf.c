
// Scheduler
#include "sst_conf.h"

#include "board.h"
#include "app_airserenity.h"
#include "app_clock.h"
#include "esp32.h"
#include "time_server.h"
#include "uart1.h"
#include "data_storage.h"

// TCP/IP
#include "tcp_client.h"

// C library        
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


// Event queue, one per task
static SSTEvent TaskAppSerenityQueue[10];
static SSTEvent TaskTcpIpQueue[10];

AirParam_t AirParam;

#define CONSOLE_RCV_BUF_SIZE  1024

static unsigned char consoleRcvBuf[CONSOLE_RCV_BUF_SIZE];
static uint32_t consoleRcvSize = 0;
static uint8_t gEndOfFrameTimerId = 0U;

void HandleUart1Rx(uint8_t data)
{
    if (consoleRcvSize < CONSOLE_RCV_BUF_SIZE)
    {
        consoleRcvBuf[consoleRcvSize] = data;
        consoleRcvSize++;
        TimerStart(gEndOfFrameTimerId); // restart timer from zero each time we receive data
   //     GpioDebug = 1;
    }
}

// Called when uart data has been received
void Console_ReceiveSignal(void)
{
  //  GpioDebug = 0;
    SST_Post(TASK_APP_SERENITY, RUN_SIG, APP_SIG_CONSOLE_DATA);
}


// =============================================================================
// FONCTIONS DU MODULE
// =============================================================================
void TASK_AppSerenity(SSTEvent e)
{
   switch (e.sig) {
      case INIT_SIG:

         
         break;
       
      // Called each second
      case RUN_SIG:
        if ((e.par == APP_SIG_LONG_PUSH) || (e.par == APP_SIG_SHORT_PUSH))
        {
            APP_GestionBoutons(e.par);
        }
        else if (e.par == APP_SIG_CONSOLE_DATA)
        {
            // On envoie la donn�e re�u pour traitement
            APP_ReceptionConsole(consoleRcvBuf, consoleRcvSize);
            consoleRcvSize = 0U;
        }

        APP_SequencePrincipale();
		break;
		
	default:
		break;
   }
}


void TASK_Initialize(void)
{
    BOARD_Initialize();
    // Ok alors on va lire tr�s tr�s t�t les param�tres manuf 
    // Comme cela, on consid�re que AirParam est initialis� pour toutes les 
    // T�ches, au pire avec des valeurs par d�faut
    ReadAirParam(&AirParam);
    
    APP_Initialize();

    // On utilise le port s�rie en interruption, pour la r�ception uniquement
    UART1_EnableRxIT();
    // 50ms end of receive detection
    gEndOfFrameTimerId = TimerInit(50, Console_ReceiveSignal);
    
    // Initialisation des t�ches
    SST_Task(&TASK_AppSerenity, TASK_APP_SERENITY, TaskAppSerenityQueue, sizeof(TaskAppSerenityQueue)/sizeof(TaskAppSerenityQueue[0]), INIT_SIG, 0);   
    SST_Task(&net_task, TASK_TCP_IP, TaskTcpIpQueue, sizeof(TaskTcpIpQueue)/sizeof(TaskTcpIpQueue[0]), INIT_SIG, 0);   

}

void TASK_TopSecond(void)
{
    CLOCK_Update();
    
    APP_TopSecond();
    BOARD_TopSecond();
    
    SST_Post(TASK_APP_SERENITY, RUN_SIG, 0);
    SST_Post(TASK_TCP_IP, RUN_SIG, 0);
}


void TASK_EmergencyStop(void)
{
    // FIXME: impl�menter quelque chose lorsque tout va mal (interrptions non g�r�es, d�passement m�moire ...)
    // Id�alement:
    //   1. Mettre un flag en m�moire non volatile (RAM non intialis�e)
    //   2. Red�marrer
    //   3. Au d�marrage suivant, d�tecter ce flag et logguer l'erreur (date, compteur, etc.)
}

