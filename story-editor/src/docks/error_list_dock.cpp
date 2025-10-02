#include "error_list_dock.h"
#include "imgui.h"

void ErrorListDock::Draw() {
    WindowBase::BeginDraw();
 
    
    ImGui::SetWindowSize(ImVec2(800, 200), ImGuiCond_FirstUseEver);
    
    // Header avec compteur
    size_t errorCount = GetErrorCount();
    size_t warningCount = GetWarningCount();
    
    if (errorCount > 0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
        ImGui::Text("%s %zu", ICON_FA_TIMES_CIRCLE, errorCount);
        ImGui::PopStyleColor();
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s 0", ICON_FA_TIMES_CIRCLE);
    }
    
    ImGui::SameLine();
    if (warningCount > 0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.0f, 1.0f));
        ImGui::Text("%s %zu", ICON_FA_EXCLAMATION_TRIANGLE, warningCount);
        ImGui::PopStyleColor();
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s 0", ICON_FA_EXCLAMATION_TRIANGLE);
    }
    
    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();
    
    if (ImGui::Button(ICON_FA_TRASH " Clear")) {
        Clear();
    }
    
    ImGui::Separator();
    
    // AJOUT du BeginChild pour la zone scrollable
    ImGui::BeginChild("ErrorListContent", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    
    // Afficher un message si pas d'erreurs
    if (m_errors.empty()) {
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), 
            "%s No errors or warnings", ICON_FA_CHECK_CIRCLE);
    } else {
        // Table des erreurs
        if (ImGui::BeginTable("ErrorTable", 3, 
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | 
            ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
        {
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Node", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableHeadersRow();
            
            for (size_t i = 0; i < m_errors.size(); ++i) {
                const auto& error = m_errors[i];
                
                ImGui::TableNextRow();
                
                // Type column
                ImGui::TableSetColumnIndex(0);
                ImGui::PushStyleColor(ImGuiCol_Text, error.GetTypeColor());
                ImGui::Text("%s %s", error.GetTypeIcon().c_str(), 
                    error.type == CompilationError::ERROR ? "Error" :
                    error.type == CompilationError::WARNING ? "Warning" : "Info");
                ImGui::PopStyleColor();
                
                // Message column
                ImGui::TableSetColumnIndex(1);
                ImGui::TextWrapped("%s", error.message.c_str());
                
                // Node column (clickable to navigate)
                ImGui::TableSetColumnIndex(2);
                if (!error.nodeId.empty()) {
                    if (ImGui::SmallButton(("Go##" + std::to_string(i)).c_str())) {
                        // TODO: Emit event to navigate to node
                    }
                    ImGui::SameLine();
                    ImGui::TextDisabled("?");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Node: %s", error.nodeId.c_str());
                    }
                } else {
                    ImGui::TextDisabled("-");
                }
            }
            
            ImGui::EndTable();
        }
    }
    
    ImGui::EndChild();
    
    WindowBase::EndDraw();
}