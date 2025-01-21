
/*
Define the NOMINMAX symbol at the top of your source, before you include any headers.
Visual C++ defines min and max as macros somewhere in windows.h, and they interfere with
your use of the corresponding standard functions.
*/
#define NOMINMAX
#include "gui.h"

#include <stdio.h>
#include <iostream>
#include <filesystem>


#include "imgui_internal.h"
// SDL2 ---------------------------------
// #include "imgui_impl_sdl2.h"
// #include "imgui_impl_sdlrenderer2.h"
// #include <SDL2/SDL.h>
// #if defined(IMGUI_IMPL_OPENGL_ES2)
// #include <SDL2/SDL_opengles2.h>
// #else
// #include <SDL2/SDL_opengl.h>
//#endif

// SDL3 ---------------------------------
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include <SDL3/SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif
#include <SDL3_image/SDL_image.h>

#include "IconsMaterialDesignIcons.h"
#include "IconsFontAwesome5_c.h"

#include "serializers.h"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static SDL_Window* window{nullptr};
static SDL_Renderer* renderer{nullptr};


#include "stb_image.h"
#include "sys_lib.h"


typedef struct {
   uint16_t type;                 /* Magic identifier            */
   uint32_t size;                       /* File size in bytes          */
   uint16_t reserved1;
   uint16_t reserved2;
   uint32_t offset;                     /* Offset to image data, bytes */
} bmp_header_t;

typedef struct {
   uint32_t size;               /* Header size in bytes      */
   uint32_t width;
   uint32_t height;                /* Width and height of image */
   uint16_t planes;       /* Number of colour planes   */
   uint16_t bits;         /* Bits per pixel            */
   uint32_t compression;        /* Compression type          */
   uint32_t imagesize;          /* Image size in bytes       */
   uint32_t xresolution;
   uint32_t yresolution;     /* Pixels per meter          */
   uint32_t ncolours;           /* Number of colours         */
   uint32_t importantcolours;   /* Important colours         */
   uint32_t rgb;
   uint32_t rgb2;
} bmp_infoheader_t;

static const uint32_t HEADER_SIZE = 14;
static const uint32_t INFO_HEADER_SIZE = 40;


uint8_t parse_bmp(const uint8_t *data, bmp_header_t *header, bmp_infoheader_t *info_header)
{
    uint8_t isBmp = 0;


    // Header is 14 bytes length
    isBmp = (data[0] == 'B') && (data[1] == 'M') ? 1 : 0;
    header->size = leu32_get(data + 2);
    header->offset = leu32_get(data + 10);

    isBmp = isBmp & 1;
    info_header->size = leu32_get(data + HEADER_SIZE);
    info_header->width = leu32_get(data + HEADER_SIZE + 4);
    info_header->height = leu32_get(data + HEADER_SIZE + 8);
    info_header->planes = leu16_get(data + HEADER_SIZE + 12);
    info_header->bits = leu16_get(data + HEADER_SIZE + 14);
    info_header->compression = leu32_get(data + HEADER_SIZE + 16);
    info_header->imagesize = leu32_get(data + HEADER_SIZE + 20);
    info_header->xresolution = leu32_get(data + HEADER_SIZE + 24);
    info_header->yresolution = leu32_get(data + HEADER_SIZE + 28);
    info_header->ncolours = leu32_get(data + HEADER_SIZE + 32);
    info_header->importantcolours = leu32_get(data + HEADER_SIZE + 36);
    info_header->rgb = leu32_get(data + HEADER_SIZE + 40);
    info_header->rgb2 = leu32_get(data + HEADER_SIZE + 44);

    return isBmp;
}


// Code de décompression
void Decompress(uint8_t *bmpImage, uint32_t fileSize, SDL_Texture *texture)
{
    uint32_t width = 320;
    uint32_t height = 240;

    bmp_header_t header;
    bmp_infoheader_t info_header;
    parse_bmp(bmpImage, &header, &info_header);

    uint8_t *compressed = &bmpImage[header.offset];
    uint32_t compressedSize = fileSize - header.offset;

    uint32_t paletteSize = header.offset - (HEADER_SIZE + INFO_HEADER_SIZE);

#ifdef DEBUG_BITMAP
    printf("File size (from header):%d\r\n", (uint32_t)header.size);
    printf("File size (from data):%d\r\n", (uint32_t)fileSize);
    printf("Data offset:%d\r\n", (uint32_t)header.offset);
    printf("Image size:%d\r\n", (uint32_t)info_header.size);
    printf("width:%d\r\n", (uint32_t)info_header.width);
    printf("height:%d\r\n", (uint32_t)info_header.height);
    printf("Planes:%d\r\n", (uint32_t)info_header.planes);
    printf("Bits:%d\r\n", (uint32_t)info_header.bits);
    printf("Compression:%d\r\n", (uint32_t)info_header.compression); // 2 - 4 bit run length encoding
    printf("Image size:%d\r\n", (uint32_t)info_header.imagesize);
    printf("X resolution:%d\r\n", (uint32_t)info_header.xresolution);
    printf("Y resolution:%d\r\n", (uint32_t)info_header.yresolution);
    printf("Colors:%d\r\n", (uint32_t)info_header.ncolours);
    printf("Important colors:%d\r\n", (uint32_t)info_header.importantcolours);
    printf("RGB :%d\r\n", (uint32_t)info_header.rgb);
    printf("RGB2 :%d\r\n", (uint32_t)info_header.rgb2);
#endif
    uint8_t *palette = &bmpImage[HEADER_SIZE + INFO_HEADER_SIZE]; // 16 * 4 bytes = 64

    // buffer de sortie, bitmap décompressé
    uint8_t decompressed[320 * 240];
    memset(decompressed, 0, 320*240);

  //  btea((uint32_t*) bmpImage, -128, key);

    uint32_t pixel = 0; // specify the pixel offset
    uint32_t i = 0;
    do
    {
        uint8_t rleCmd = compressed[i];
        if (rleCmd > 0)
        {
            uint8_t val = compressed[i + 1];
            // repeat number of pixels
            for (uint32_t j = 0; j < rleCmd; j++)
            {
                if ((j & 1) == 0)
                {
                    decompressed[pixel] = (val & 0xF0) >>4;
                }
                else
                {
                    decompressed[pixel] = (val & 0x0F);
                }
                pixel++;
            }
            i += 2; // jump pair instruction
        }
        else
        {
            uint8_t second = compressed[i + 1];
            if (second == 0)
            {
                if (pixel % width)
                {
                    // end of line
                    uint32_t lines = pixel / width;
                    uint32_t remaining = width - (pixel - (lines * width));

                    pixel += remaining;
                }
                i += 2;
            }
            else if (second == 1)
            {
                // std::cout << "End of bitmap" << std::endl;
                goto end;
            }
            else if (second == 2)
            {
                // delta N pixels and M lines
                pixel += compressed[i + 2] + compressed[i + 3] * width;
                i += 4;
            }
            else
            {
                // absolute mode
                uint8_t *ptr = &compressed[i + 2];
                // repeat number of pixels
                for (uint32_t j = 0; j < second; j++)
                {
                    if ((j & 1) == 0)
                    {
                        decompressed[pixel] = (*ptr & 0xF0) >> 4;
                    }
                    else
                    {
                        decompressed[pixel] = (*ptr & 0x0F);
                        ptr++;
                    }
                    pixel++;
                }
                i += 2 + (second / 2);

                // padded in word boundary, jump if necessary
                if ((second / 2) % 2)
                {
                    i++;
                }
            }
        }

        if (pixel > (width * height))
        {
            std::cout << "Error!" << std::endl;
        }
    }
    while( i < compressedSize);

end:
     void *pixels;
    int pitch;
    SDL_LockTexture(texture, NULL, &pixels, &pitch);

    // Définir la texture comme cible de rendu
    SDL_SetRenderTarget(renderer, texture);


    uint32_t x = 0, y = 0;
    for (uint32_t i = 0; i < pixel; i++)
    {
        uint8_t val = decompressed[i];
        if (val > 15)
        {
            std::cout << "Error!" << std::endl;
        }

        uint8_t *palettePtr = &palette[val * 4];
        // Définir la couleur de rendu
        SDL_SetRenderDrawColor(renderer, palettePtr[0], palettePtr[1], palettePtr[2], 255);

        int flippedY = (height - 1) - y;
        // Dessiner un point à la position (x, y)
        SDL_RenderPoint(renderer, x, flippedY);

        x++;
        if (x >= width)
        {
            x = 0;
            y++;
        }
    }

    SDL_SetRenderTarget(renderer, NULL);
    SDL_UnlockTexture(texture);

  

//    std::ofstream outfile ("new.txt", std::ofstream::binary);
//    outfile.write (reinterpret_cast<const char *>(decompressed), pixel / 2 );
//    outfile.close();

}



// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char* filename, Gui::Image &img)
{
    SDL_Surface *surface, *temp;
    bool useSdlImage = true;

    if (SysLib::GetFileExtension(filename) == "bmp")
    {
        // BMP
        bmp_header_t header;
        bmp_infoheader_t info_header;
        uint8_t *data = nullptr;
        size_t size = 0;


        FILE *fil;
        uint32_t offset;
        fil = fopen(filename, "r");

        // Lire tout le ficher
        fseek(fil, 0, SEEK_END);
        size = ftell(fil);
        fseek(fil, 0, SEEK_SET);

        uint8_t *bmpImage = (uint8_t *)malloc(size);
        fread(bmpImage, 1, size, fil);

        parse_bmp(bmpImage, &header, &info_header);

        if (info_header.bits == 4)
        {
            auto tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, info_header.width, info_header.height);
            Decompress(bmpImage, size, tex);
            useSdlImage = false;

            img.texture = tex;
            img.w = info_header.width;
            img.h = info_header.height;
        }

        free(bmpImage);
    }

    if (useSdlImage)
    {

        surface = IMG_Load(filename);
        if (!surface)
        {
            SDL_Log("Couldn't load %s: %s\n", filename, SDL_GetError());
            return false;
        }

        /* Use the tonemap operator to convert to SDR output */
        // const char *tonemap = NULL;
        // SDL_SetStringProperty(SDL_GetSurfaceProperties(surface), SDL_PROP_SURFACE_TONEMAP_OPERATOR_STRING, tonemap);
        temp = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
        SDL_DestroySurface(surface);
        if (!temp)
        {
            SDL_Log("Couldn't convert surface: %s\n", SDL_GetError());
            return false;
        }

        img.texture = SDL_CreateTextureFromSurface(renderer, temp);
        SDL_DestroySurface(temp);
        if (!img.texture) {
            SDL_Log("Couldn't create texture: %s\n", SDL_GetError());
        return false;
        }

        float w, h = 0;
        SDL_GetTextureSize(static_cast<SDL_Texture*>(img.texture), &w, &h);
        img.w = (int)w;
        img.h = (int)h;
    }

    return true;
}


std::string GetDirectory (const std::string& path)
{
    size_t found = path.find_last_of("/\\");
    return(path.substr(0, found));
}

Gui::Gui()
{
    m_executablePath = std::filesystem::current_path().generic_string();
    std::cout << "PATH: " << m_executablePath << std::endl;
}

static ImFont* normalFont;
// static ImFont* bigFont;



void Gui::CopyToClipboard(const std::string &text)
{
    SDL_SetClipboardText(text.c_str());
}


void Gui::PushBigFont()
{
    // ImGui::PushFont(bigFont);
}

void Gui::PopBigFont()
{
    // ImGui::PopFont();
}

bool Gui::Initialize()
{
    // Setup SDL2
    // if (SDL_Init(SDL_INIT_EVERYTHING) != 0)

    // Setup SDL3
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO) < 0)
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return false;
    }
/*
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
    {
        int i, num_devices;
        SDL_AudioDeviceID *devices = SDL_GetAudioOutputDevices(&num_devices);
        if (devices) {
            for (i = 0; i < num_devices; ++i) {
                SDL_AudioDeviceID instance_id = devices[i];
                char *name = SDL_GetAudioDeviceName(instance_id);
                SDL_Log("AudioDevice %" SDL_PRIu32 ": %s\n", instance_id, name);
                SDL_free(name);
            }
            SDL_free(devices);
        }
        
        printf("Error: SDL_InitSubSystem(): %s\n", SDL_GetError());
        return false;
    }*/

    // Enable native IME.
    // SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

    // Create window with SDL_Renderer graphics context
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);

    // SDL3
    window = SDL_CreateWindow("Story Editor", 1280, 720, window_flags);

    // SDL2
    // window = SDL_CreateWindow("Story Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);

    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return false;
    }
    renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer == nullptr)
    {
        SDL_Log("Error: SDL_CreateRenderer(): %s\n", SDL_GetError());
        return false;
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
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;


    // bigFont = io.Fonts->AddFontFromFileTTF( std::string(m_executablePath + "/fonts/roboto.ttf").c_str(), 40);
    // io.Fonts->Build();
    normalFont = io.Fonts->AddFontFromFileTTF( std::string(m_executablePath + "/fonts/roboto.ttf").c_str(), 20);

    {
        ImFontConfig config;
        config.MergeMode = true; // ATTENTION, MERGE AVEC LA FONT PRECEDENTE !!
        //    config.GlyphMinAdvanceX = 20.0f; // Use if you want to make the icon monospaced
        //    config.GlyphOffset.y += 1.0;
        static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
        io.Fonts->AddFontFromFileTTF(std::string(m_executablePath + "/fonts/fa-solid-900.ttf").c_str(), 16.0f, &config, icon_ranges);

        io.Fonts->Build();
    }

    {
        ImFontConfig config;
        config.MergeMode = true; // ATTENTION, MERGE AVEC LA FONT PRECEDENTE !!

        static const ImWchar icon_ranges_mdi[] = { ICON_MIN_MDI, ICON_MAX_MDI, 0 };
        io.Fonts->AddFontFromFileTTF(std::string(m_executablePath + "/fonts/materialdesignicons-webfont.ttf").c_str(), 16.0f, &config, icon_ranges_mdi);

        io.Fonts->Build();
    }
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends

    // SDL3
   ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
   ImGui_ImplSDLRenderer3_Init(renderer);

    // SDL2
    // ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    // ImGui_ImplSDLRenderer2_Init(renderer);

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

        ImGui_ImplSDL3_ProcessEvent(&event);
        if (event.type == SDL_EVENT_QUIT)
            done = true;
        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
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

void Gui::StartFrame()
{
    // Start the Dear ImGui frame

    // SDL3
   ImGui_ImplSDLRenderer3_NewFrame();
   ImGui_ImplSDL3_NewFrame();

    // SDL2
    // ImGui_ImplSDLRenderer2_NewFrame();
    // ImGui_ImplSDL2_NewFrame();

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

    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer); // SDL3
    // ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData()); // SDL2

    SDL_RenderPresent(renderer);
}

void Gui::Destroy()
{
    // Cleanup

    // SDL3
   ImGui_ImplSDLRenderer3_Shutdown();
   ImGui_ImplSDL3_Shutdown();

    // SDL2
    // ImGui_ImplSDLRenderer2_Shutdown();
    // ImGui_ImplSDL2_Shutdown();


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

    if (std::filesystem::is_regular_file(filename))
    {
        LoadTextureFromFile(filename.c_str(), image);
    }

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
