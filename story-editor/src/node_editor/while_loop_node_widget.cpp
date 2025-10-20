// story-editor/src/node_editor/while_loop_node_widget.cpp
#include "while_loop_node_widget.h"
#include "imgui.h"
#include "IconsMaterialDesignIcons.h"

WhileLoopNodeWidget::WhileLoopNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> base)
    : BaseNodeWidget(manager, base)
{
    m_whileLoopNode = std::dynamic_pointer_cast<WhileLoopNode>(base);
}

void WhileLoopNodeWidget::Draw()
{
    // Affichage simple
    ImGui::Text(ICON_MDI_REPEAT " While Loop");
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
    ImGui::TextWrapped("Loops while condition is true");
    ImGui::PopStyleColor();
}

void WhileLoopNodeWidget::DrawProperties(std::shared_ptr<IStoryProject> story)
{
    ImGui::Text("While Loop Properties");
    ImGui::Separator();
    
    ImGui::TextWrapped("The loop executes as long as the condition input evaluates to true (non-zero).");
    ImGui::Spacing();
    ImGui::TextWrapped("Warning: Ensure the condition can become false to avoid infinite loops.");
}