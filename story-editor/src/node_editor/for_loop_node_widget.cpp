// story-editor/src/node_editor/for_loop_node_widget.cpp
#include "for_loop_node_widget.h"
#include "imgui.h"
#include "IconsMaterialDesignIcons.h"

ForLoopNodeWidget::ForLoopNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> base)
    : BaseNodeWidget(manager, base)
{
    m_forLoopNode = std::dynamic_pointer_cast<ForLoopNode>(base);
}

void ForLoopNodeWidget::Draw()
{
    // Affichage simple du nœud dans l'éditeur
    ImGui::Text(ICON_MDI_SYNC " For Loop");
    
    // Afficher les valeurs configurées (si non connectées)
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
    ImGui::TextWrapped("Start: %d, End: %d, Step: %d", 
                       m_forLoopNode->GetStartIndex(),
                       m_forLoopNode->GetEndIndex(),
                       m_forLoopNode->GetStep());
    ImGui::PopStyleColor();
}

void ForLoopNodeWidget::DrawProperties(std::shared_ptr<IStoryProject> story)
{
    ImGui::Text("For Loop Properties");
    ImGui::Separator();
    
    // Configuration des valeurs par défaut
    int start = m_forLoopNode->GetStartIndex();
    if (ImGui::InputInt("Start Index", &start)) {
        m_forLoopNode->SetStartIndex(start);
    }
    
    int end = m_forLoopNode->GetEndIndex();
    if (ImGui::InputInt("End Index", &end)) {
        m_forLoopNode->SetEndIndex(end);
    }
    
    int step = m_forLoopNode->GetStep();
    if (ImGui::InputInt("Step", &step)) {
        if (step != 0) { // Éviter division par zéro
            m_forLoopNode->SetStep(step);
        }
    }
    
    ImGui::Separator();
    ImGui::TextWrapped("Note: These values are used only if the corresponding input ports are not connected.");
}
