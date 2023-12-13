#include <iostream>
#include <memory>
#include "imgui.h"
#include "IconsFontAwesome5.h"
#include "SDL.h"
#include "imgui-knobs.h"

#include "ost_common.h"
#include "ost_wrapper.h"
#include "picture.h"

static SDL_Texture *display_texture{nullptr};


// OST stubs
extern "C" void system_putc(char ch)
{
    std::cout << ch;
    std::flush(std::cout);
}


static color_t image_buffer[320][240];

extern "C" void ost_display_draw_h_line(uint16_t y, uint8_t *pixels, uint8_t *palette)
{
    color_t color;

    for(uint16_t i = 0; i < 320; i++)
    {
        uint8_t val = pixels[i];

        if (val > 15)
        {
            val = 0;
        }

        const uint8_t *palettePtr = &palette[val * 4];
        color.r = palettePtr[0];
        color.g = palettePtr[1];
        color.b = palettePtr[2];

        image_buffer[i][y] = color;
    }
}



static void ost_impl_show_image(SDL_Renderer *renderer, const char *fileName)
{
    decompress(fileName);

    display_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_TARGET, 320, 240);

    // Switch the renderer to the texture
    SDL_SetRenderTarget(renderer, display_texture);
    SDL_RenderClear(renderer);

    for (uint16_t y = 0; y < 240; y++)
    {
        for(uint16_t x = 0; x < 320; x++)
        {
            SDL_SetRenderDrawColor(renderer, image_buffer[x][y].r, image_buffer[x][y].g, image_buffer[x][y].b, 255);
            SDL_RenderDrawPoint(renderer, x, 239 - y);
        }
    }

    SDL_RenderPresent(renderer);

    // Switch back to main screen renderer
    SDL_SetRenderTarget(renderer, NULL);
}


// Emulated hardware
void draw_ost_device(SDL_Renderer *renderer, double deltaTime)
{
    static int load = 0;

    if (load == 0)
    {
        load = 1;
        ost_impl_show_image(renderer, "assets/images/example.bmp");
    }


    ImGui::Begin("OST device");

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    if (ImGui::Button("OK"))
    {
        std::cout << "OK button clicked" << std::endl;
    }
    ImGui::SameLine();
    if (ImGui::Button("Home"))
    {
        std::cout << "Choose RIGHT" << std::endl;
    }
    ImGui::SameLine();
    if (ImGui::Button("Pause"))
    {
        std::cout << "Choose RIGHT" << std::endl;
    }
    ImGui::SameLine();
    if (ImGui::Button("Rotate LEFT"))
    {
        std::cout << "Choose LEFT" << std::endl;
    }
    ImGui::SameLine();
    if (ImGui::Button("Rotate RIGHT"))
    {
        std::cout << "Choose RIGHT" << std::endl;
    }

    ImGui::NewLine();

    // Contenu de l'écran

    if (display_texture != nullptr)
    {
        ImGui::Image(display_texture, ImVec2((float)320, (float)240));
    }
    else
    {
        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
        ImVec2 canvas_size(320, 240);

        // Rectangle vide
        draw_list->AddRectFilledMultiColor(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(50, 50, 50, 255), IM_COL32(50, 50, 60, 255), IM_COL32(60, 60, 70, 255), IM_COL32(50, 50, 60, 255));
        // contour blanc
        draw_list->AddRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(255, 255, 255, 255));
    }

    ImGui::End();
}


