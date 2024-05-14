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
        void *texture{nullptr}; // Platform specific
        int w;
        int h;

        bool Valid() const {
            return (w && h);
        }

        void Clear();

        void Load(const std::string &filename);

        Image();
        ~Image();

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
    void SetWindowTitle(const std::string &title);

    static bool LoadRawImage(const std::string &filename, Image &image);
    static Size GetWindowSize();

    static void PushBigFont();
    static void PopBigFont();

private:
    std::string m_executablePath;
    
};

// namespace ImGui {
// void LoadingIndicatorCircle(const char* label, const float indicator_radius,
//                                    const ImVec4& main_color, const ImVec4& backdrop_color,
//                                    const int circle_count, const float speed);
// }

#endif // GUI_H
