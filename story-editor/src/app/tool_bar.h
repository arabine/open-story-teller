// toolbar.h
#ifndef TOOL_BAR_H
#define TOOL_BAR_H

#include "imgui.h" // Nécessaire pour ImGui::Begin, ImGui::Button, etc.
#include "app_controller.h" // Supposons que AppController gère les actions
#include "IconsMaterialDesignIcons.h" // Pour les icônes (si vous utilisez le même jeu d'icônes)

class ToolBar
{
public:
    // Le constructeur prend une référence au contrôleur d'application.
    // Cela permet à la barre d'outils de déclencher des actions métier.
    ToolBar(AppController& appController)
        : m_appController(appController) {}

    // Dessine la barre d'outils.
    // 'topPadding' est utile si vous avez une barre de menu horizontale au-dessus.
    void Draw(float topPadding)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + topPadding), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetFrameHeightWithSpacing() * 1.5f, viewport->Size.y - topPadding - ImGui::GetFrameHeight()), ImGuiCond_Always);

        // Pas de barre de titre, pas de redimensionnement, pas de mouvement, etc.
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.f, 5.f)); // Petit padding
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f); // Pas d'arrondi
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f); // Pas de bordure
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 10.f)); // Espacement vertical entre les boutons

        if (ImGui::Begin("ToolBar", nullptr, windowFlags))
        {
            float buttonSize = ImGui::GetContentRegionAvail().x; // Remplir la largeur disponible

            // Boutons d'action : Ouvrir, Sauvegarder, Compiler, etc.
            // Utilisez des icônes pour une meilleure UX.
            // Assurez-vous que les icônes sont chargées dans la police ImGui.

            ImGui::PushID("ToolBarButtons"); // Empêche les ID de collision si plusieurs boutons ont le même texte

            // Bouton Nouveau
            if (ImGui::Button(ICON_MDI_FILE_OUTLINE " New", ImVec2(buttonSize, buttonSize))) {
                m_appController.NewStory(); // Supposons que AppController ait une méthode NewStory()
            }
            ImGui::Tooltip("New Story");

            // Bouton Ouvrir
            if (ImGui::Button(ICON_MDI_FOLDER_OPEN_OUTLINE " Open", ImVec2(buttonSize, buttonSize))) {
                // Ici, vous pourriez déclencher une boîte de dialogue d'ouverture de fichier via AppController
                // Ou, si la boîte de dialogue est une fenêtre ImGui séparée, MainWindow la gèrerait.
                // Pour simplifier, nous supposons AppController gère le dialogue ou déclenche un événement.
                m_appController.OpenStory();
            }
            ImGui::Tooltip("Open Story");

            // Bouton Sauvegarder
            if (ImGui::Button(ICON_MDI_CONTENT_SAVE_OUTLINE " Save", ImVec2(buttonSize, buttonSize))) {
                m_appController.SaveStory();
            }
            ImGui::Tooltip("Save Story");

            ImGui::Separator(); // Séparateur visuel

            // Bouton Compiler
            if (ImGui::Button(ICON_MDI_CODE_BRACES " Compile", ImVec2(buttonSize, buttonSize))) {
                m_appController.Build(true); // Appelle la méthode de compilation dans AppController
            }
            ImGui::Tooltip("Compile Code");

            // Bouton Exécuter/Build
            if (ImGui::Button(ICON_MDI_PLAY_OUTLINE " Run", ImVec2(buttonSize, buttonSize))) {
                m_appController.Build(false); // Appelle la méthode de build et exécution
            }
            ImGui::Tooltip("Build and Run");

            // Bouton Stop VM
            if (ImGui::Button(ICON_MDI_STOP " Stop", ImVec2(buttonSize, buttonSize))) {
                m_appController.Stop(); // Arrête la VM
            }
            ImGui::Tooltip("Stop VM");

            // Bouton Pas à pas (Debugger)
            if (ImGui::Button(ICON_MDI_STEP_FORWARD " Step", ImVec2(buttonSize, buttonSize))) {
                m_appController.Step(); // Exécute une instruction
            }
            ImGui::Tooltip("Step VM Instruction");


            ImGui::PopID(); // Pop l'ID pushé
        }
        ImGui::End();
        ImGui::PopStyleVar(4); // Pop les 4 styles poussés
    }

private:
    AppController& m_appController; // Référence au contrôleur d'application
};

#endif // TOOL_BAR_H