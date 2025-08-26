
#include <sstream>
#include "operator_node_widget.h"

namespace ed = ax::NodeEditor;
#include "IconsMaterialDesignIcons.h"
#include "story_project.h"
#include "uuid.h"

OperatorNodeWidget::OperatorNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
    : BaseNodeWidget(manager, node)
    , m_manager(manager)
{
    m_opNode = std::dynamic_pointer_cast<OperatorNode>(node);

    // Create defaut one input and one output
    AddInputs(2);
    SetInputPinName(0, "A");
    SetInputPinName(1, "B");
    AddOutputs(1);
    SetOutPinName(0, "=");
}

void OperatorNodeWidget::Initialize()
{
    BaseNodeWidget::Initialize();
}



void OperatorNodeWidget::DrawProperties(std::shared_ptr<IStoryProject> story)
{
    ImGui::AlignTextToFramePadding();
    static ImGuiComboFlags flags = 0;
    
    if (ImGui::BeginCombo("Operators", m_selectedOperatorSymbol.c_str(), flags))
    {
        int i = 0;

        for (auto& op : m_opNode->GetOperators())
        {
                       // ImGui::PushID(static_cast<int>(i)); // Assure l'unicité des widgets
            if (op.first == m_selectedOperatorType)
            {
                m_selectedIndex = i;
            }

            const bool is_selected = (m_selectedIndex == i);

            if (ImGui::Selectable(op.second.symbol.c_str(), is_selected))
            {
                m_selectedIndex = i;
                m_selectedOperatorSymbol = op.second.symbol;
                m_opNode->SetOperationType(op.first);
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }

            i++;
        }
        ImGui::EndCombo();
    }

}

void OperatorNodeWidget::Draw()
{
    BaseNodeWidget::FrameStart();

    ImGui::TextUnformatted(m_selectedOperatorSymbol.c_str());

    DrawPins();

    BaseNodeWidget::FrameEnd();

}

