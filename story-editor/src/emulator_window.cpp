#include "emulator_window.h"
#include "gui.h"
#include "ImGuiFileDialog.h"
#include "IconsMaterialDesignIcons.h"

EmulatorWindow::EmulatorWindow(IStoryManager &proj)
    : WindowBase("Emulator")
    , m_story(proj)
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

    if (m_image.Valid())
    {
        ImGui::Image(m_image.texture, ImVec2(320, 240));
    }
    else
    {
        ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + 320, p.y + 240), ImGui::GetColorU32(ImVec4(1.0, 1.0, 1.0, 1.0)));
    }

    ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + 244));

    float old_size = ImGui::GetFont()->Scale;
    ImGui::GetFont()->Scale *= 2;

    ImGui::PushFont(ImGui::GetFont());

    if (ImGui::Button(ICON_MDI_CHECK_CIRCLE_OUTLINE, ImVec2(50, 50)))
    {
        m_story.Ok();
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_MDI_STOP_CIRCLE_OUTLINE, ImVec2(50, 50)))
    {
        m_story.Pause();
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_MDI_ARROW_LEFT_BOLD_CIRCLE_OUTLINE, ImVec2(50, 50)))
    {
        m_story.Previous();
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_MDI_ARROW_RIGHT_BOLD_CIRCLE_OUTLINE, ImVec2(50, 50)))
    {
        m_story.Next();
    }

    ImGui::GetFont()->Scale = old_size;
    ImGui::PopFont();

    ImGui::SeparatorText("Script control and debug");

    if (ImGui::Button("Build nodes"))
    {
        m_story.BuildNodes(true);
    }
    ImGui::SameLine();
    if (ImGui::Button("Build code"))
    {
        m_story.BuildCode(true);
    }
    ImGui::SameLine();
    if (ImGui::Button("Play"))
    {
        m_story.Log("Play story");
        m_story.Play();
    }

    ImGui::SameLine();
    if (ImGui::Button("Stop"))
    {
        m_story.Log("Stop story");
        m_story.Stop();
    }

    ImGui::SameLine();

    ImGui::Text("VM state: %s", m_story.VmState().c_str());


    if (ImGui::Button("Load binary story (.c32)"))
    {
        IGFD::FileDialogConfig config;
        config.path = ".";
        config.countSelectionMax = 1;
        config.flags = ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog("LoadBinarySoryDlgKey", 
            "Choose a binary story", 
            ".c32",
            config
        );
    }

     // ---------------- Load Binary story
    if (ImGuiFileDialog::Instance()->Display("LoadBinarySoryDlgKey"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            std::string filter = ImGuiFileDialog::Instance()->GetCurrentFilter();
            
            m_story.LoadBinaryStory(filePathName);
        }
        // close
        ImGuiFileDialog::Instance()->Close();
    }



    WindowBase::EndDraw();
}

void EmulatorWindow::ClearImage()
{
    m_image.Clear();
}

void EmulatorWindow::SetImage(const std::string &image)
{
    m_imageFileName = image;
    m_image.Load(m_story.BuildFullAssetsPath(image));
}
