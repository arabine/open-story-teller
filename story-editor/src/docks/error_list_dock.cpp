#include "error_list_dock.h"
#include "imgui.h"

void ErrorListDock::Draw() {
    // Auto-show when errors are added
    if (m_shouldShow) {
        ImGui::SetNextWindowFocus();
        m_shouldShow = false; // Reset flag after showing once
    }
    
    WindowBase::BeginDraw();
    
    ImGui::SetWindowSize(ImVec2(800, 250), ImGuiCond_FirstUseEver);
    
    // Header avec compteur et barre de couleur selon le type d'erreur
    size_t errorCount = GetErrorCount();
    size_t warningCount = GetWarningCount();
    
    // Barre de statut colorée en haut
    if (errorCount > 0) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.9f, 0.2f, 0.2f, 0.15f));
    } else if (warningCount > 0) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 0.8f, 0.0f, 0.15f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.3f, 0.8f, 0.3f, 0.15f));
    }
    
    ImGui::BeginChild("StatusBar", ImVec2(0, 42), true);
    
    // Errors count
    if (errorCount > 0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
        ImGui::Text("%s %zu Error%s", ICON_FA_TIMES_CIRCLE, errorCount, errorCount > 1 ? "s" : "");
        ImGui::PopStyleColor();
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s 0 Errors", ICON_FA_TIMES_CIRCLE);
    }
    
    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();
    
    // Warnings count
    if (warningCount > 0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.0f, 1.0f));
        ImGui::Text("%s %zu Warning%s", ICON_FA_EXCLAMATION_TRIANGLE, warningCount, warningCount > 1 ? "s" : "");
        ImGui::PopStyleColor();
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s 0 Warnings", ICON_FA_EXCLAMATION_TRIANGLE);
    }
    
    // Clear button on the right
    ImGui::SameLine(ImGui::GetWindowWidth() - 100);
    if (ImGui::Button(ICON_FA_TRASH " Clear")) {
        Clear();
    }
    
    ImGui::EndChild();
    ImGui::PopStyleColor(); // StatusBar background
    
    ImGui::Separator();
    
    // Main content area with scrolling
    ImGui::BeginChild("ErrorListContent", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    
    // Afficher un message si pas d'erreurs
    if (m_errors.empty()) {
        ImGui::Dummy(ImVec2(0, 20)); // Spacing
        ImGui::Indent(20);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
        ImGui::Text("%s", ICON_FA_CHECK_CIRCLE);
        ImGui::SameLine();
        ImGui::TextWrapped("No compilation errors or warnings. Your code is ready to run!");
        ImGui::PopStyleColor();
        ImGui::Unindent(20);
    } else {
        // Table des erreurs avec colonnes redimensionnables
        if (ImGui::BeginTable("ErrorTable", 4, 
            ImGuiTableFlags_Borders | 
            ImGuiTableFlags_RowBg | 
            ImGuiTableFlags_Resizable | 
            ImGuiTableFlags_ScrollY |
            ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("##icon", ImGuiTableColumnFlags_WidthFixed, 30);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Line", ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableHeadersRow();
            
            for (size_t i = 0; i < m_errors.size(); ++i) {
                const auto& error = m_errors[i];
                
                ImGui::TableNextRow();
                ImGui::PushID(static_cast<int>(i));
                
                // Icon column
                ImGui::TableSetColumnIndex(0);
                ImGui::PushStyleColor(ImGuiCol_Text, error.GetTypeColor());
                ImGui::Text("%s", error.GetTypeIcon().c_str());
                ImGui::PopStyleColor();
                
                // Type column
                ImGui::TableSetColumnIndex(1);
                ImGui::PushStyleColor(ImGuiCol_Text, error.GetTypeColor());
                const char* typeStr = (error.type == CompilationError::ERROR) ? "Error" :
                                     (error.type == CompilationError::WARNING) ? "Warning" : "Info";
                ImGui::Text("%s", typeStr);
                ImGui::PopStyleColor();
                
                // Message column
                ImGui::TableSetColumnIndex(2);
                ImGui::TextWrapped("%s", error.message.c_str());
                
                // Line column
                ImGui::TableSetColumnIndex(3);
                if (error.line > 0) {
                    ImGui::Text("%d", error.line);
                } else {
                    ImGui::TextDisabled("-");
                }
                
                // Optional: Node navigation button
                if (!error.nodeId.empty()) {
                    ImGui::SameLine();
                    if (ImGui::SmallButton(ICON_FA_ARROW_RIGHT)) {
                        // TODO: Emit event to navigate to node
                        // m_eventBus.Emit(NavigateToNodeEvent(error.nodeId));
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Go to node: %s", error.nodeId.c_str());
                    }
                }
                
                ImGui::PopID();
            }
            
            ImGui::EndTable();
        }
    }
    
    ImGui::EndChild();
    
    WindowBase::EndDraw();
}