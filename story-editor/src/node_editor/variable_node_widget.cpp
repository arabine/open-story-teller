
#include <sstream>
#include "variable_node_widget.h"

#include "IconsMaterialDesignIcons.h"
#include "story_project.h"
#include "uuid.h"

VariableNodeWidget::VariableNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
    : BaseNodeWidget(manager, node)
    , m_manager(manager)
{
    m_variableNode = std::dynamic_pointer_cast<VariableNode>(node);
    SetTitle("Variable");
}

void VariableNodeWidget::Initialize()
{
    BaseNodeWidget::Initialize();

    m_selectedVariableUuid = m_variableNode->GetVariableUuid();
}


void VariableNodeWidget::DrawProperties(std::shared_ptr<IStoryProject> story)
{
    if (!m_isInitialized)
    {
        m_isInitialized = true;
        story->ScanVariable([this] (std::shared_ptr<Variable> var) {
            if (var->GetUuid() == m_selectedVariableUuid)
            {
                m_selectedVariableName = var->GetVariableName();
            }
        });
    }

    ImGui::AlignTextToFramePadding();
    static ImGuiComboFlags flags = 0;

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);

    if (ImGui::BeginCombo("Variables list", m_selectedVariableName.c_str(), flags))
    {
        int i = 0;
        story->ScanVariable([&i, this] (std::shared_ptr<Variable> var) {

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
    ImGui::TextUnformatted(m_selectedVariableName.c_str());
}

