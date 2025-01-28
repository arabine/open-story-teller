
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
    // Create defaut one input and one output
    //AddInput();
    AddOutputs(1);
    SetOutPinName(0, "");
}

void VariableNodeWidget::Initialize()
{
    BaseNodeWidget::Initialize();
}



void VariableNodeWidget::DrawProperties()
{
    ImGui::AlignTextToFramePadding();
    static ImGuiComboFlags flags = 0;

    if (ImGui::BeginCombo("Variables list", m_selectedVariable.c_str(), flags))
    {
        int i = 0;
        m_manager.ScanVariable([&i, this] (Variable &var) {

            // ImGui::PushID(static_cast<int>(i)); // Assure l'unicité des widgets

            const bool is_selected = (m_selectedIndex == i);
            if (ImGui::Selectable(var.name.c_str(), is_selected))
            {
                m_selectedIndex = i;
                m_selectedVariable = var.name;
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        });
        ImGui::EndCombo();
    }

}

void VariableNodeWidget::Draw()
{
    BaseNodeWidget::FrameStart();

    ImGui::TextUnformatted(m_selectedVariable.c_str());

    DrawPins();

    BaseNodeWidget::FrameEnd();

}

