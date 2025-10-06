#include "emulator_dock.h"
#include "gui.h"
#include "ImGuiFileDialog.h"
#include "IconsMaterialDesignIcons.h"

EmulatorDock::EmulatorDock(IStoryManager &proj)
    : WindowBase("Emulator")
    , m_story(proj)
{
}

void EmulatorDock::Initialize()
{
}

void EmulatorDock::AddVmOutput(const std::string& text)
{
    std::scoped_lock<std::mutex> lock(m_vmOutputMutex);
    m_vmOutput.push_back({text, 0});
    
    // Limiter à 1000 lignes
    if (m_vmOutput.size() > 1000) {
        m_vmOutput.erase(m_vmOutput.begin());
    }
}

void EmulatorDock::ClearVmOutput()
{
    std::scoped_lock<std::mutex> lock(m_vmOutputMutex);
    m_vmOutput.clear();
}

void EmulatorDock::Draw()
{
    WindowBase::BeginDraw();
    ImGui::SetWindowSize(ImVec2(626, 744), ImGuiCond_FirstUseEver);

    // ===== ÉCRAN DE L'ÉMULATEUR =====
    ImVec2 p = ImGui::GetCursorScreenPos();

    if (m_image.Valid())
    {
        ImGui::Image(reinterpret_cast<ImTextureID>(m_image.texture), ImVec2(320, 240));
    }
    else
    {
        ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + 320, p.y + 240), 
            ImGui::GetColorU32(ImVec4(0.1f, 0.1f, 0.1f, 1.0f)));
        
        // Texte centré "No Image"
        ImGui::SetCursorScreenPos(ImVec2(p.x + 110, p.y + 110));
        ImGui::TextDisabled("No Image");
    }

    ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + 244));

    // ===== BOUTONS DE CONTRÔLE =====
    float old_size = ImGui::GetFont()->Scale;
    ImGui::GetFont()->Scale *= 2;
    ImGui::PushFont(ImGui::GetFont());

    if (ImGui::Button(ICON_MDI_CHECK_CIRCLE_OUTLINE, ImVec2(50, 50)))
    {
        m_story.Ok();
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("OK");
    
    ImGui::SameLine();
    if (ImGui::Button(ICON_MDI_PAUSE, ImVec2(50, 50)))
    {
        m_story.Pause();
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Pause");
    
    ImGui::SameLine();
    if (ImGui::Button(ICON_MDI_HOME, ImVec2(50, 50)))
    {
        m_story.Home();
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Home");
    
    ImGui::SameLine();
    if (ImGui::Button(ICON_MDI_ARROW_LEFT_BOLD_CIRCLE_OUTLINE, ImVec2(50, 50)))
    {
        m_story.Previous();
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Previous");
    
    ImGui::SameLine();
    if (ImGui::Button(ICON_MDI_ARROW_RIGHT_BOLD_CIRCLE_OUTLINE, ImVec2(50, 50)))
    {
        m_story.Next();
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Next");

    ImGui::GetFont()->Scale = old_size;
    ImGui::PopFont();

    // ===== CONTRÔLES DE SCRIPT ET DEBUG =====
    ImGui::SeparatorText("Script control and debug");

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

    // ===== BOUTONS DE CHARGEMENT =====
    if (ImGui::Button("Load binary story (.c32)"))
    {
        IGFD::FileDialogConfig config;
        config.path = ".";
        config.countSelectionMax = 1;
        config.flags = ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog("LoadBinaryStoryDlgKey", 
            "Choose a binary story", 
            ".c32",
            config
        );
    }

    ImGui::SameLine();
    if (ImGui::Button("Set script source file (.chip32)"))
    {
        IGFD::FileDialogConfig config;
        config.path = ".";
        config.countSelectionMax = 1;
        config.flags = ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog("SetSourceScriptDlgKey", 
            "Choose a source file", 
            ".chip32",
            config
        );
    }

    // ===== DIALOGS =====
    // Load Binary story
    if (ImGuiFileDialog::Instance()->Display("LoadBinaryStoryDlgKey"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            m_story.LoadBinaryStory(filePathName);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    // External source file
    if (ImGuiFileDialog::Instance()->Display("SetSourceScriptDlgKey"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            m_story.SetExternalSourceFile(filePathName);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    // ===== CONSOLE VM (NOUVEAU) =====
    DrawVmConsole();

    WindowBase::EndDraw();
}

void EmulatorDock::DrawVmConsole()
{
    ImGui::Separator();
    
    // Header de la console
    ImGui::Text(ICON_MDI_CONSOLE " VM Output");
    ImGui::SameLine();
    
    ImGui::Checkbox("Auto-scroll##vm", &m_autoScrollVm);
    ImGui::SameLine();
    
    if (ImGui::SmallButton("Clear##vm"))
    {
        ClearVmOutput();
    }
    
    ImGui::Separator();
    
    // Zone de console scrollable
    ImGui::BeginChild("VmConsoleRegion", ImVec2(0, 150), true, 
        ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);
    
    {
        std::scoped_lock<std::mutex> lock(m_vmOutputMutex);
        
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2));
        ImGui::PushFont(ImGui::GetFont()); // Police monospace si disponible
        
        if (m_vmOutput.empty())
        {
            ImGui::TextDisabled("(No VM output yet - run the program to see print statements)");
        }
        else
        {
            for (const auto& entry : m_vmOutput)
            {
                // Colorer les erreurs en rouge
                if (entry.type == 1) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
                }
                
                ImGui::TextUnformatted(entry.text.c_str());
                
                if (entry.type == 1) {
                    ImGui::PopStyleColor();
                }
            }
        }
        
        ImGui::PopFont();
        ImGui::PopStyleVar();
    }
    
    // Auto-scroll intelligent (comme la console principale)
    float scrollY = ImGui::GetScrollY();
    float scrollMaxY = ImGui::GetScrollMaxY();
    const float BOTTOM_THRESHOLD = 5.0f;
    bool wasAtBottom = (scrollMaxY == 0) || (scrollY >= scrollMaxY - BOTTOM_THRESHOLD);
    
    if (m_autoScrollVm && wasAtBottom)
    {
        ImGui::SetScrollHereY(1.0f);
    }
    
    ImGui::EndChild();
}

void EmulatorDock::ClearImage()
{
    m_image.Clear();
}

void EmulatorDock::SetImage(const std::string &image)
{
    m_imageFileName = image;
    m_image.Load(m_story.BuildFullAssetsPath(image));
}