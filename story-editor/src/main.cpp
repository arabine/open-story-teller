
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

    auto w = std::make_shared<MainWindow>(logger, eventBus, appController);

    bool done = false;

    appController.Initialize();
    if (w->Initialize())
    {
        while (!done)
        {
            done = w->Loop();
            appController.ProcessStory();
        }
    }

    return 0;
}
