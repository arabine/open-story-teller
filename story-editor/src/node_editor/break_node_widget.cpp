#include "break_node_widget.h"
#include "imgui.h"
#include "IconsMaterialDesignIcons.h"

BreakNodeWidget::BreakNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> base)
    : BaseNodeWidget(manager, base)
{
    m_breakNode = std::dynamic_pointer_cast<BreakNode>(base);
}

void BreakNodeWidget::Draw()
{
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
    ImGui::Text(ICON_MDI_STOP " Break");
    ImGui::PopStyleColor();
}

void BreakNodeWidget::DrawProperties(std::shared_ptr<IStoryProject> story)
{
    ImGui::Text("Break Statement");
    ImGui::Separator();
    ImGui::TextWrapped("Exits the current loop immediately.");
    ImGui::TextWrapped("Must be placed inside a For or While loop.");
}
