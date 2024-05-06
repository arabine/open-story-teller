
#include "display.h"

#include <stdio.h>
#include <iostream>
#include <filesystem>
#include "math.h"


#include <SDL3/SDL.h>
#ifdef ANDROID
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif
#include <SDL3_image/SDL_image.h>

using namespace gfx;

static SDL_Window* gWindow{nullptr};
static SDL_Renderer* renderer{nullptr};
static SDL_Colour gBackgroundColor{14, 23, 29, 255};



Display::Display()
{
    m_executablePath = SDL_GetBasePath();
    std::cout << "PATH: " << m_executablePath << std::endl;
}


bool Display::Initialize(const std::string &title)
{
    // Setup SDL2
    // if (SDL_Init(SDL_INIT_EVERYTHING) != 0)

    // Setup SDL3
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMEPAD) != 0)
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return -1;
    }

    // Enable native IME.
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

    // Create window with SDL_Renderer graphics context
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL  

#ifdef ANDROID
                                                     | SDL_WINDOW_FULLSCREEN
#endif
    | SDL_WINDOW_HIDDEN);

    // SDL3
    gWindow = SDL_CreateWindow(title.c_str(), mMinimumWidth, mMinimumHeight, window_flags);

    // SDL2
    // window = SDL_CreateWindow("Story Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);


    // limit to which minimum size user can resize the window
    SDL_SetWindowMinimumSize(gWindow, mMinimumWidth, mMinimumHeight);


    if (gWindow == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    renderer = SDL_CreateRenderer(gWindow, nullptr, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        SDL_Log("Error: SDL_CreateRenderer(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetWindowPosition(gWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(gWindow);


    int w, h;
    SDL_GetWindowSize(gWindow, &w, &h);

    mScreenWidth = static_cast<uint32_t>(w);
    mScreenHeight = static_cast<uint32_t>(h);
    mRatioW = w / mWidth;
    mRatioH = h / mHeight;

    mRatio = sqrt(mRatioW * mRatioW + mRatioH * mRatioH); // pow(GetSystem().GetRatioH(), .8) * pow(GetSystem().GetRatioW(), 0.2);


   
    // SDL2
    // ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    // ImGui_ImplSDLRenderer2_Init(renderer);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImDisplay::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    return true;
}


bool Display::PollEvent()
{
    bool done = false;
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        // SDL3
        if (event.type == SDL_EVENT_QUIT)
            done = true;
        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(gWindow))
            done = true;

        // SLD2
        // ImGui_ImplSDL2_ProcessEvent(&event);
        // if (event.type == SDL_QUIT)
        //     done = true;
        // if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
        //     done = true;

    }
    return done;

}

void Display::StartFrame()
{

}

void Display::EndFrame()
{
    //SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    SDL_SetRenderDrawColor(renderer, (Uint8)(0.45f * 255), (Uint8)(0.55f * 255), (Uint8)(0.60f * 255), (Uint8)(1.00f * 255));
    SDL_RenderClear(renderer);

    SDL_RenderPresent(renderer);
}

void Display::Destroy()
{
    // Cleanup

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
}

void Display::SetWindowTitle(const std::string &title)
{
    SDL_SetWindowTitle(gWindow, title.c_str());
}


Display::Size Display::GetWindowSize()
{
    Size s;
    SDL_GetWindowSize(gWindow, &s.w, &s.h);
    return s;
}
