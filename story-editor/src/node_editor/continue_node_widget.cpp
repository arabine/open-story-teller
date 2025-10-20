#include "continue_node_widget.h"
#include "imgui.h"
#include "IconsMaterialDesignIcons.h"

ContinueNodeWidget::ContinueNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> base)
    : BaseNodeWidget(manager, base)
{
    m_continueNode = std::dynamic_pointer_cast<ContinueNode>(base);
}

void ContinueNodeWidget::Draw()
{
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));
    ImGui::Text(ICON_MDI_SKIP_NEXT " Continue");
    ImGui::PopStyleColor();
}

void ContinueNodeWidget::DrawProperties(std::shared_ptr<IStoryProject> story)
{
    ImGui::Text("Continue Statement");
    ImGui::Separator();
    ImGui::TextWrapped("Skips to the next iteration of the current loop.");
    ImGui::TextWrapped("Must be placed inside a For or While loop.");
}