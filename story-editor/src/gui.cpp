
/*
Define the NOMINMAX symbol at the top of your source, before you include any headers.
Visual C++ defines min and max as macros somewhere in windows.h, and they interfere with
your use of the corresponding standard functions.
*/
#define NOMINMAX
#include "gui.h"

#include <stdio.h>


#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <stdio.h>
#include <SDL2/SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL2/SDL_opengl.h>
#endif

#include "IconsMaterialDesignIcons.h"
#include "IconsFontAwesome5_c.h"
#include "qoi.h"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static SDL_Window* window{nullptr};
static SDL_Renderer* renderer{nullptr};


#include "stb_image.h"

static std::string GetFileExtension(const std::string &fileName)
{
    if(fileName.find_last_of(".") != std::string::npos)
        return fileName.substr(fileName.find_last_of(".")+1);
    return "";
}


// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char* filename, Gui::Image &img)
{


    std::string ext = GetFileExtension(filename);

    SDL_Surface* surface = nullptr;

    if (ext == "qoi")
    {
        qoi_desc desc;
        void *pixels = qoi_read(filename, &desc, 0);
        unsigned int channels = desc.channels;
        img.w = desc.width;
        img.h = desc.height;

        surface = SDL_CreateRGBSurfaceFrom((void*)pixels, img.w, img.h, channels * 8, channels * img.w,
                                           0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

        if (pixels != NULL)
        {
            free(pixels);
        }
    }
    else
    {
        unsigned char* data = nullptr;
        int channels;
        data = stbi_load(filename, &img.w, &img.h, &channels, 0);

        if (data == nullptr) {
            fprintf(stderr, "Failed to load image: %s\n", stbi_failure_reason());
            return false;
        }


        // SDL3
        //    SDL_Surface* surface = SDL_CreateSurfaceFrom((void*)data, img.w, img.h, 4 * img.w, SDL_PIXELFORMAT_RGBA8888);

        // SDL2
        surface = SDL_CreateRGBSurfaceFrom((void*)data, img.w, img.h, channels * 8, channels * img.w,
                                           0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
        stbi_image_free(data);
    }


    if (surface == nullptr) {
        fprintf(stderr, "Failed to create SDL surface: %s\n", SDL_GetError());
        return false;
    }

    img.texture = SDL_CreateTextureFromSurface(renderer, surface);

    if (img.texture == nullptr) {
        fprintf(stderr, "Failed to create SDL texture: %s\n", SDL_GetError());
    }

//    SDL_DestroySurface(surface); // SDL3
    SDL_FreeSurface(surface); // SDL2


    return true;
}

#define MANOLAB_VERSION "1.0"

Gui::Gui()
{

}


bool Gui::Initialize()
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return -1;
    }

    // Enable native IME.
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

    // Create window with SDL_Renderer graphics context
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);

    // SDL3
    //window = SDL_CreateWindow("Dear ImGui SDL3+SDL_Renderer example", 1280, 720, window_flags);

    // SDL2
    window = SDL_CreateWindow("Story Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);

    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        SDL_Log("Error: SDL_CreateRenderer(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);


    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;


    io.Fonts->AddFontFromFileTTF( "fonts/roboto.ttf", 20);

    {
        ImFontConfig config;
        config.MergeMode = true; // ATTENTION, MERGE AVEC LA FONT PRECEDENTE !!
        //    config.GlyphMinAdvanceX = 20.0f; // Use if you want to make the icon monospaced
        //    config.GlyphOffset.y += 1.0;
        static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
        io.Fonts->AddFontFromFileTTF("fonts/fa-solid-900.ttf", 16.0f, &config, icon_ranges);

        io.Fonts->Build();
    }

    {
        ImFontConfig config;
        config.MergeMode = true; // ATTENTION, MERGE AVEC LA FONT PRECEDENTE !!

        static const ImWchar icon_ranges_mdi[] = { ICON_MIN_MDI, ICON_MAX_MDI, 0 };
        io.Fonts->AddFontFromFileTTF("fonts/materialdesignicons-webfont.ttf", 16.0f, &config, icon_ranges_mdi);

        io.Fonts->Build();
    }
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends

    // SDL3
//    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
//    ImGui_ImplSDLRenderer3_Init(renderer);


    // SDL2
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
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


bool Gui::PollEvent()
{
    bool done = false;
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        // SDL3
        /*
        ImGui_ImplSDL3_ProcessEvent(&event);
        if (event.type == SDL_EVENT_QUIT)
            done = true;
        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
            done = true;
*/

        // SLD2
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT)
            done = true;
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
            done = true;

    }
    return done;

}

void Gui::StartFrame()
{
    // Start the Dear ImGui frame

    // SDL3
//    ImGui_ImplSDLRenderer3_NewFrame();
//    ImGui_ImplSDL3_NewFrame();

    // SDL2
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();

    ImGui::NewFrame();
}

void Gui::EndFrame()
{
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    // Rendering
    // Rendering

    //SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
    SDL_RenderClear(renderer);

    ImGui::Render();

    //ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData()); // SDL3
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData()); // SDL2

    SDL_RenderPresent(renderer);
}

void Gui::Destroy()
{
    // Cleanup

    // SDL3
//    ImGui_ImplSDLRenderer3_Shutdown();
//    ImGui_ImplSDL3_Shutdown();

    // SDL2
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();


    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Gui::SetWindowTitle(const std::string &title)
{
    SDL_SetWindowTitle(window, title.c_str());
}

bool Gui::LoadRawImage(const std::string &filename, Image &image)
{
    bool success = true;

    LoadTextureFromFile(filename.c_str(), image);

    return success;
}

Gui::Size Gui::GetWindowSize()
{
    Size s;
    SDL_GetWindowSize(window, &s.w, &s.h);
    return s;
}

void Gui::ApplyTheme()
{
    ImVec4* colors = ImGui::GetStyle().Colors;
    ImGuiStyle & style = ImGui::GetStyle();


    colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.44f, 0.44f, 0.44f, 0.60f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.57f, 0.57f, 0.57f, 0.70f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.76f, 0.76f, 0.76f, 0.80f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.13f, 0.75f, 0.55f, 0.80f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.13f, 0.75f, 0.75f, 0.80f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
    colors[ImGuiCol_Button]                 = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
    colors[ImGuiCol_Header]                 = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
    colors[ImGuiCol_Separator]              = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.13f, 0.75f, 0.55f, 0.80f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.13f, 0.75f, 0.75f, 0.80f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.36f, 0.36f, 0.36f, 0.54f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);


    style.ChildRounding = 4.0f;
    style.FrameBorderSize = 1.0f;
    style.FrameRounding = 2.0f;
    style.GrabMinSize = 7.0f;
    style.PopupRounding = 2.0f;
    style.ScrollbarRounding = 12.0f;
    style.ScrollbarSize = 13.0f;
    style.TabBorderSize = 1.0f;
    style.TabRounding = 0.0f;
    style.WindowRounding = 4.0f;

/*
        /// 0 = FLAT APPEARENCE
        /// 1 = MORE "3D" LOOK
        int is3D = 1;

        colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled]           = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_ChildBg]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_WindowBg]               = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_PopupBg]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_Border]                 = ImVec4(0.12f, 0.12f, 0.12f, 0.71f);
        colors[ImGuiCol_BorderShadow]           = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        colors[ImGuiCol_FrameBg]                = ImVec4(0.42f, 0.42f, 0.42f, 0.54f);
        colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.42f, 0.42f, 0.42f, 0.40f);
        colors[ImGuiCol_FrameBgActive]          = ImVec4(0.56f, 0.56f, 0.56f, 0.67f);
        colors[ImGuiCol_TitleBg]                = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.17f, 0.17f, 0.17f, 0.90f);
        colors[ImGuiCol_MenuBarBg]              = ImVec4(0.335f, 0.335f, 0.335f, 1.000f);
        colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.24f, 0.24f, 0.24f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
        colors[ImGuiCol_CheckMark]              = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
        colors[ImGuiCol_SliderGrab]             = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
        colors[ImGuiCol_Button]                 = ImVec4(0.54f, 0.54f, 0.54f, 0.35f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(0.52f, 0.52f, 0.52f, 0.59f);
        colors[ImGuiCol_ButtonActive]           = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
        colors[ImGuiCol_Header]                 = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
        colors[ImGuiCol_HeaderHovered]          = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
        colors[ImGuiCol_HeaderActive]           = ImVec4(0.76f, 0.76f, 0.76f, 0.77f);
        colors[ImGuiCol_Separator]              = ImVec4(0.000f, 0.000f, 0.000f, 0.137f);
        colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.700f, 0.671f, 0.600f, 0.290f);
        colors[ImGuiCol_SeparatorActive]        = ImVec4(0.702f, 0.671f, 0.600f, 0.674f);
        colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
        colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.73f, 0.73f, 0.73f, 0.35f);
        colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
        colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

        style.PopupRounding = 3;

        style.WindowPadding = ImVec2(4, 4);
        style.FramePadding  = ImVec2(6, 4);
        style.ItemSpacing   = ImVec2(6, 2);

        style.ScrollbarSize = 18;

        style.WindowBorderSize = 1;
        style.ChildBorderSize  = 1;
        style.PopupBorderSize  = 1;
        style.FrameBorderSize  = is3D;

        style.WindowRounding    = 3;
        style.ChildRounding     = 3;
        style.FrameRounding     = 3;
        style.ScrollbarRounding = 2;
        style.GrabRounding      = 3;


        style.TabBorderSize = is3D;
        style.TabRounding   = 3;

        colors[ImGuiCol_DockingEmptyBg]     = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
        colors[ImGuiCol_Tab]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_TabHovered]         = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_TabActive]          = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
        colors[ImGuiCol_TabUnfocused]       = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
        colors[ImGuiCol_DockingPreview]     = ImVec4(0.85f, 0.85f, 0.85f, 0.28f);

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        */

}



#include "imgui_internal.h"
namespace ImGui {
void LoadingIndicatorCircle(const char* label, const float indicator_radius,
                                   const ImVec4& main_color, const ImVec4& backdrop_color,
                                   const int circle_count, const float speed) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) {
        return;
    }

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(label);

    const ImVec2 pos = window->DC.CursorPos;
    const float circle_radius = indicator_radius / 10.0f;
    const ImRect bb(pos, ImVec2(pos.x + indicator_radius * 2.0f,
                                pos.y + indicator_radius * 2.0f));
    ItemSize(bb, GetStyle().FramePadding.y);
    if (!ItemAdd(bb, id)) {
        return;
    }
    const float t = g.Time;
    const auto degree_offset = 2.0f * IM_PI / circle_count;
    for (int i = 0; i < circle_count; ++i) {
        const auto x = indicator_radius * std::sin(degree_offset * i);
        const auto y = indicator_radius * std::cos(degree_offset * i);
        const auto growth = std::max(0.0f, std::sin(t * speed - i * degree_offset));
        ImVec4 color;
        color.x = main_color.x * growth + backdrop_color.x * (1.0f - growth);
        color.y = main_color.y * growth + backdrop_color.y * (1.0f - growth);
        color.z = main_color.z * growth + backdrop_color.z * (1.0f - growth);
        color.w = 1.0f;
        window->DrawList->AddCircleFilled(ImVec2(pos.x + indicator_radius + x,
                                                 pos.y + indicator_radius - y),
                                                 circle_radius + growth * circle_radius,
                                                 GetColorU32(color));
    }
}
} // namespace ImGui

void Gui::Image::Clear()
{
    if (texture != nullptr)
    {
        SDL_DestroyTexture(static_cast<SDL_Texture*>(texture));
    }
}

void Gui::Image::Load(const std::string &filename)
{
    Clear();
    Gui::LoadRawImage(filename, *this);
}

Gui::Image::Image()
{
    texture = nullptr;
    w = 0;
    h = 0;
}

Gui::Image::~Image()
{
    Clear();
}
