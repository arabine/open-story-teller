// story-editor/src/node_editor/wait_delay_node_widget.cpp
#include "wait_delay_node_widget.h"
#include "IconsMaterialDesignIcons.h"

WaitDelayNodeWidget::WaitDelayNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
    : BaseNodeWidget(manager, node)
{
    m_waitDelayNode = std::dynamic_pointer_cast<WaitDelayNode>(node);
}

void WaitDelayNodeWidget::Initialize()
{
    BaseNodeWidget::Initialize();
}

void WaitDelayNodeWidget::DrawProperties(std::shared_ptr<IStoryProject> story)
{
    ImGui::AlignTextToFramePadding();
    
    int duration = static_cast<int>(m_waitDelayNode->GetDuration());
    if (ImGui::InputInt("Duration (ms)", &duration, 10, 100))
    {
        if (duration < 0) duration = 0;
        m_waitDelayNode->SetDuration(static_cast<uint32_t>(duration));
    }
    
    ImGui::Spacing();
    ImGui::TextDisabled("Pauses execution for the specified duration");
}

void WaitDelayNodeWidget::Draw()
{
    uint32_t duration = m_waitDelayNode->GetDuration();
    
    ImGui::Text(ICON_MDI_TIMER_OUTLINE " Wait");
    
    if (duration >= 1000) {
        ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), 
                          "%.1f sec", duration / 1000.0f);
    } else {
        ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), 
                          "%u ms", duration);
    }
}