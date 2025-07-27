// about_dialog.h
#ifndef ABOUT_DIALOG_H
#define ABOUT_DIALOG_H

#include "imgui.h" // Nécessaire pour ImGui::BeginPopupModal, ImGui::Text, etc.
#include <string>   // Pour std::string
#include "library_manager.h" // Pour LibraryManager::GetVersion()
#include <SDL3/SDL.h> // Pour SDL_GetPlatform(), SDL_GetNumLogicalCPUCores(), SDL_GetSystemRAM()

// Il est important de noter que cette classe AboutDialog
// dépendra de ImGui et de SDL pour afficher les informations
// système, comme c'est le cas dans le code original de MainWindow.

class AboutDialog
{
public:
    AboutDialog() = default;

    // Affiche le popup "About".
    // Doit être appelée à chaque frame dans la boucle de rendu ImGui.
    void Draw()
    {
        // Toujours centrer cette fenêtre lorsqu'elle apparaît
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("AboutPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Story Editor - v%s", LibraryManager::GetVersion().c_str());
            ImGui::Text("http://www.openstoryteller.org");
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Platform");

            ImGui::Text("%s", SDL_GetPlatform());
            ImGui::Text("CPU cores: %d", SDL_GetNumLogicalCPUCores());
            ImGui::Text("RAM: %.2f GB", SDL_GetSystemRAM() / 1024.0f);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Separator();

            ImGui::SameLine(300);
            if (ImGui::Button("Close", ImVec2(100, 35)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    // Ouvre le popup "About".
    // Doit être appelée lorsque l'utilisateur clique sur "About" dans le menu.
    void Open()
    {
        if (m_showAboutPopup)
        {
            ImGui::OpenPopup("AboutPopup");
        }
    }

    void Show()
    {
        m_showAboutPopup = true;
    }

    void Reset()
    {
        m_showAboutPopup = false;
    }

private:
    bool m_showAboutPopup = false; // Indique si le popup "About" doit être affiché
};

#endif // ABOUT_DIALOG_H