#ifndef GUI_H
#define GUI_H

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"

#include <string>

class Gui
{
public:
    Gui();

    struct Texture;

    struct Image {
        void *texture; // Platform specific
        int w;
        int h;

        std::string name;

        bool valid() const {
            return (w && h);
        }

        Image();

    };

    struct Size {
        int w;
        int h;
    };

    void ApplyTheme();
    bool Initialize();
    bool PollEvent();
    void StartFrame();
    void EndFrame();
    void Destroy();

    static bool LoadRawImage(const std::string &filename, Image &image);
    static Size GetWindowSize();

private:

};

namespace ImGui {
void LoadingIndicatorCircle(const char* label, const float indicator_radius,
                                   const ImVec4& main_color, const ImVec4& backdrop_color,
                                   const int circle_count, const float speed);
}

#endif // GUI_H
