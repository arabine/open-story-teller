#include "emulator_window.h"
#include "gui.h"

EmulatorWindow::EmulatorWindow()
    : WindowBase("Emulator")
{

    Gui::LoadRawImage("assets/play.png", m_playImage);
}

void EmulatorWindow::Initialize() {

    int my_image_width = 0;
    int my_image_height = 0;

}

void EmulatorWindow::Draw()
{
//    if (!IsVisible())
//    {
//        return;
//    }

    static ImVec2 size(320.0f, 240.0f);

    WindowBase::BeginDraw();
    ImGui::SetWindowSize(ImVec2(626, 744), ImGuiCond_FirstUseEver);

//    ImGui::Image((void*)(intptr_t)my_image_texture, ImVec2(313, 367));

//    float sz = ImGui::GetTextLineHeight();
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + 320, p.y + 240), ImGui::GetColorU32(ImVec4(1.0, 1.0, 1.0, 1.0)));

    ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + 240));
    ImGui::ImageButton("play", m_playImage.texture, ImVec2(45, 45));

    WindowBase::EndDraw();
}
