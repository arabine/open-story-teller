#include "emulator_window.h"
#include "gui.h"
#include "IconsMaterialDesignIcons.h"

EmulatorWindow::EmulatorWindow(IStoryProject &proj)
    : WindowBase("Emulator")
    , m_project(proj)
{

}

void EmulatorWindow::Initialize()
{



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

    ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + 244));

    float old_size = ImGui::GetFont()->Scale;
    ImGui::GetFont()->Scale *= 2;

    ImGui::PushFont(ImGui::GetFont());


    ImGui::Button(ICON_MDI_PLAY_CIRCLE_OUTLINE, ImVec2(50, 50));
    ImGui::SameLine();
    ImGui::Button(ICON_MDI_STOP_CIRCLE_OUTLINE, ImVec2(50, 50));
    ImGui::SameLine();
    ImGui::Button(ICON_MDI_ARROW_LEFT_BOLD_CIRCLE_OUTLINE, ImVec2(50, 50));
    ImGui::SameLine();
    ImGui::Button(ICON_MDI_ARROW_RIGHT_BOLD_CIRCLE_OUTLINE, ImVec2(50, 50));

    ImGui::GetFont()->Scale = old_size;
    ImGui::PopFont();

    ImGui::SeparatorText("Script control and debug");

    if (ImGui::Button("Build"))
    {
        m_project.Build();
    }
    ImGui::SameLine();

    WindowBase::EndDraw();
}
