
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
            // On envoie la donnée reçu pour traitement
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
    // Ok alors on va lire très très tôt les paramètres manuf 
    // Comme cela, on considère que AirParam est initialisé pour toutes les 
    // Tâches, au pire avec des valeurs par défaut
    ReadAirParam(&AirParam);
    
    APP_Initialize();

    // On utilise le port série en interruption, pour la réception uniquement
    UART1_EnableRxIT();
    // 50ms end of receive detection
    gEndOfFrameTimerId = TimerInit(50, Console_ReceiveSignal);
    
    // Initialisation des tâches
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
    // FIXME: implémenter quelque chose lorsque tout va mal (interrptions non gérées, dépassement mémoire ...)
    // Idéalement:
    //   1. Mettre un flag en mémoire non volatile (RAM non intialisée)
    //   2. Redémarrer
    //   3. Au démarrage suivant, détecter ce flag et logguer l'erreur (date, compteur, etc.)
}

