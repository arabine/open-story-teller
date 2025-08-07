
#include "main_window.h"
#include "app_controller.h"
#include "event_bus.h"
#include "logger.h"

// Main code
int main(int, char**)
{
    Logger logger;
    EventBus eventBus;

    AppController appController(logger, eventBus);
    
    MainWindow w(logger, eventBus, appController);
    
    if (w.Initialize())
    {
        w.Loop();
        appController.ProcessStory();
    }  

    return 0;
}
