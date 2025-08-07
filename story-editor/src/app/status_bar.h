// status_bar.h
#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#include "imgui.h" // Nécessaire pour ImGui::Begin, ImGui::Text, etc.
#include <string>   // Pour std::string
#include "app_controller.h" // Supposons que AppController expose les infos nécessaires

class StatusBar
{
public:
    // Le constructeur prend une référence au contrôleur d'application.
    // Cela permet à la barre de statut d'accéder aux informations qu'elle doit afficher.
    StatusBar(AppController& appController)
        : m_appController(appController) {}

    // Dessine la barre de statut.
    // Doit être appelée à chaque frame dans la boucle de rendu ImGui.
    void Draw()
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - ImGui::GetFrameHeight()), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, ImGui::GetFrameHeight()), ImGuiCond_Always);

        // Pas de barre de titre, pas de redimensionnement, pas de mouvement, etc.
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.f, 4.f)); // Petit padding
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f); // Pas d'arrondi
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f); // Pas de bordure
        if (ImGui::Begin("StatusBar", nullptr, windowFlags))
        {
            // Affiche les FPS
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            // Affiche l'état de la VM (supposons que AppController a une méthode pour cela)
            ImGui::SameLine();
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
            ImGui::SameLine();
            ImGui::Text("VM State: %s", m_appController.VmState().c_str());

            // Vous pouvez ajouter d'autres informations ici, par exemple :
            // - Nom du projet actuel
            // - État du serveur web
            // - Messages de log courts (dernier message critique, etc.)

        }
        ImGui::End();
        ImGui::PopStyleVar(3); // Pop les 3 styles poussés
    }

private:
    AppController& m_appController; // Référence au contrôleur d'application
};

#endif // STATUS_BAR_H