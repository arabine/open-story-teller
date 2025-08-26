
#include <sstream>
#include "print_node_widget.h"

namespace ed = ax::NodeEditor;
#include "IconsMaterialDesignIcons.h"
#include "story_project.h"
#include "uuid.h"

char PrintNodeWidget::m_buffer[PrintNodeWidget::MAX_PRINT_SIZE] = {0};

PrintNodeWidget::PrintNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
    : BaseNodeWidget(manager, node)
    , m_manager(manager)
{
    m_printNode = std::dynamic_pointer_cast<PrintNode>(node);
    // Create defaut one input and one output
    AddInputs(1);
    SetInputPinName(0, "");
    AddOutputs(1);
    SetOutPinName(0, "");
}

void PrintNodeWidget::Initialize()
{
    BaseNodeWidget::Initialize();
}


void PrintNodeWidget::DrawProperties(std::shared_ptr<IStoryProject> story)
{
    ImGui::AlignTextToFramePadding();

    ImGui::PushItemWidth(100.0f);

    auto t = m_printNode->GetText();
    t.copy(m_buffer, sizeof(m_buffer) - 1);

    bool edited = ImGui::InputText("##edit", m_buffer, sizeof(m_buffer), ImGuiInputTextFlags_EnterReturnsTrue);

   // if (edited)
    {
        m_printNode->SetText(m_buffer);
    }
}

void PrintNodeWidget::Draw()
{
    BaseNodeWidget::FrameStart();  

    DrawPins();

    BaseNodeWidget::FrameEnd();

}

