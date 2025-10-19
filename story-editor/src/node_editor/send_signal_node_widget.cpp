// story-editor/src/node_editor/send_signal_node_widget.cpp
#include "send_signal_node_widget.h"
#include "IconsMaterialDesignIcons.h"

SendSignalNodeWidget::SendSignalNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
    : BaseNodeWidget(manager, node)
{
    m_sendSignalNode = std::dynamic_pointer_cast<SendSignalNode>(node);
}

void SendSignalNodeWidget::Initialize()
{
    BaseNodeWidget::Initialize();
}

void SendSignalNodeWidget::DrawProperties(std::shared_ptr<IStoryProject> story)
{
    ImGui::AlignTextToFramePadding();
    
    int signalId = static_cast<int>(m_sendSignalNode->GetSignalId());
    if (ImGui::InputInt("Signal ID", &signalId, 1, 10))
    {
        if (signalId < 0) signalId = 0;
        m_sendSignalNode->SetSignalId(static_cast<uint32_t>(signalId));
    }
    
    ImGui::Spacing();
    ImGui::TextDisabled("Sends a signal to other processes");
    ImGui::TextDisabled("Signal ID: 0x%08X", m_sendSignalNode->GetSignalId());
}

void SendSignalNodeWidget::Draw()
{
    uint32_t signalId = m_sendSignalNode->GetSignalId();
    
    ImGui::Text(ICON_MDI_SIGNAL " Signal");
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), 
                      "ID: 0x%X", signalId);
}