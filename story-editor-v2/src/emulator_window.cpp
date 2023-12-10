#include "emulator_window.h"
#include "gui.h"

EmulatorWindow::EmulatorWindow()
    : WindowBase("Emulator")
{

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



    WindowBase::BeginDraw();
    ImGui::SetWindowSize(ImVec2(626, 744), ImGuiCond_FirstUseEver);

//    ImGui::Image((void*)(intptr_t)my_image_texture, ImVec2(313, 367));

    float sz = ImGui::GetTextLineHeight();
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32(ImVec4(1.0, 1.0, 1.0, 1.0)));

    WindowBase::EndDraw();
}
