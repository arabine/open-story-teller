

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "display.h"
#include "xlog.h"
#include "SDL_main.h"

// Main code
extern "C" int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
LOG_DEBUG("INIT DISPLAY");
    gfx::Display display;
    
    display.Initialize("Story player");

    LOG_DEBUG("END INIT DISPLAY");
        // Main loop
    bool done = false;

    while (!done)
    {
        uint64_t frameStart = SDL_GetTicks();
        bool aboutToClose = display.PollEvent();

        display.StartFrame();

        display.EndFrame();
        
        // Rendering and event handling
        uint64_t frameTime = SDL_GetTicks() - frameStart; // Temps écoulé pour la frame
        static const uint64_t fps = 16;
        if (frameTime < fps) { // Limite de 60 FPS
            SDL_Delay((uint32_t)(fps - frameTime)); // Attendez pour compléter la frame
        }

        if (aboutToClose)
        {
            done = true;
        }

    }

    display.Destroy();

    return 0;
}
