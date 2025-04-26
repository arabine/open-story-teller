
#include <sstream>
#include "variable_node_widget.h"

namespace ed = ax::NodeEditor;
#include "IconsMaterialDesignIcons.h"
#include "story_project.h"
#include "uuid.h"

VariableNodeWidget::VariableNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
    : BaseNodeWidget(manager, node)
    , m_manager(manager)
{
    m_variableNode = std::dynamic_pointer_cast<VariableNode>(node);
    // Create defaut one input and one output
    //AddInput();
    AddOutputs(1);
    SetOutPinName(0, "");
}

void VariableNodeWidget::Initialize()
{
    BaseNodeWidget::Initialize();

    m_selectedVariableUuid = m_variableNode->GetVariableUuid();

    m_manager.ScanVariable([this] (std::shared_ptr<Variable> var) {
        if (var->GetUuid() == m_selectedVariableUuid)
        {
            m_selectedVariableName = var->GetVariableName();
        }
    });
}


void VariableNodeWidget::DrawProperties()
{
    ImGui::AlignTextToFramePadding();
    static ImGuiComboFlags flags = 0;

    if (ImGui::BeginCombo("Variables list", m_selectedVariableName.c_str(), flags))
    {
        int i = 0;
        m_manager.ScanVariable([&i, this] (std::shared_ptr<Variable> var) {

            // ImGui::PushID(static_cast<int>(i)); // Assure l'unicité des widgets

            const bool is_selected = (m_selectedIndex == i);
            std::string l = var->GetVariableName();
            if (ImGui::Selectable(l.c_str(), is_selected))
            {
                m_selectedIndex = i;
                m_selectedVariableName = l;
                m_variableNode->SetVariableUuid(var->GetUuid());
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();

            i++;
        });
        ImGui::EndCombo();
    }

}

void VariableNodeWidget::Draw()
{
    BaseNodeWidget::FrameStart();

    ImGui::TextUnformatted(m_selectedVariableName.c_str());

    DrawPins();

    BaseNodeWidget::FrameEnd();

}

