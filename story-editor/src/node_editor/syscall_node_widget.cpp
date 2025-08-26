
#include <sstream>
#include "syscall_node_widget.h"

namespace ed = ax::NodeEditor;
#include "IconsMaterialDesignIcons.h"
#include "story_project.h"
#include "uuid.h"

SyscallNodeWidget::SyscallNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
    : BaseNodeWidget(manager, node)
{
    m_syscallNode = std::dynamic_pointer_cast<SyscallNode>(node);

    // Create defaut one input and one output
    AddInputs(4);
    SetInputPinName(0, "Arg 1");
    SetInputPinName(1, "Arg 2");
    SetInputPinName(2, "Arg 3");
    SetInputPinName(3, "Arg 4");
    AddOutputs(1);
    SetOutPinName(0, "");
}

void SyscallNodeWidget::Initialize()
{
    BaseNodeWidget::Initialize();
}

void SyscallNodeWidget::DrawProperties(std::shared_ptr<IStoryProject> story)
{
    ImGui::AlignTextToFramePadding();
    static ImGuiComboFlags flags = 0;
    
    int syscallNumber = m_syscallNode->GetSyscallNumber();
    if (ImGui::InputInt("Syscall Number", &syscallNumber, 1, 1000))
    {
        m_syscallNode->SetSyscallNumber(syscallNumber);
    }

}

void SyscallNodeWidget::Draw()
{
    BaseNodeWidget::FrameStart();

    switch(m_syscallNode->GetSyscallNumber())
    {
        case SyscallNode::SYSCALL_PLAY_MEDIA:
            ImGui::Text("Play Media");
            break;
        default:
            ImGui::Text("Unknown syscall");
            break;
    }

    DrawPins();

    BaseNodeWidget::FrameEnd();

}

