// module_properties_dialog.h
#ifndef MODULE_PROPERTIES_DIALOG_H
#define MODULE_PROPERTIES_DIALOG_H

#include "dialog_base.h"
#include <string>
#include "i_story_manager.h"
#include "IconsMaterialDesignIcons.h"

class ModulePropertiesDialog : public DialogBase
{
public:
    ModulePropertiesDialog(IStoryManager &storyManager)
        : m_storyManager(storyManager)
    {
       m_module_name[0] = '\0';
    }

    // Affiche le popup "Module Properties".
    // Doit être appelée à chaque frame dans la boucle de rendu ImGui.
    void Draw() override
    {
        auto module = m_storyManager.GetCurrentModule();
        if (module)
        {
            if (m_firstOpen)
            {
                ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                m_firstOpen = false;
                // Copy current module name to the edit buffer
                std::size_t length = module->GetName().copy(m_module_name, sizeof(m_module_name) - 1);
                m_module_name[length] = '\0';
            }
            DrawContent(module);
        }
        else
        {
            Reset();
            return;
        }
    }

private:
    IStoryManager &m_storyManager;
    char m_module_name[256] = "";

    // Implémentation de la méthode virtuelle pure GetTitle()
    const char* GetTitle() const override
    {
        return "ModulePropertiesPopup";
    }

    void DrawContent(std::shared_ptr<IStoryProject> module)
    {
        if (ImGui::BeginPopupModal(GetTitle(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text(ICON_MDI_PUZZLE_OUTLINE " Module Properties");
            ImGui::Separator();
            ImGui::Spacing();

            // Module Name
            ImGui::Text("Module name:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(300.0f);
            ImGui::InputTextWithHint("##module_name", "Enter module name", m_module_name, IM_ARRAYSIZE(m_module_name));

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Buttons
            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                // Validate and save
                if (strlen(m_module_name) > 0)
                {
                    module->SetName(m_module_name);
                    m_storyManager.SaveModule();
                    ImGui::CloseCurrentPopup();
                    Reset();
                }
                else
                {
                    // Show error: name cannot be empty
                    ImGui::OpenPopup("EmptyNameError");
                }
            }
            
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
                Reset();
            }

            // Error popup for empty name
            if (ImGui::BeginPopupModal("EmptyNameError", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text(ICON_MDI_ALERT " Module name cannot be empty!");
                ImGui::Spacing();
                if (ImGui::Button("OK", ImVec2(120, 0)))
                {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            ImGui::EndPopup();
        }
    }
};

#endif // MODULE_PROPERTIES_DIALOG_H