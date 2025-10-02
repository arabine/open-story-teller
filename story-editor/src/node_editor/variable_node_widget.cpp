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
    // Initialize variable name from UUID on first call
    if (!m_isInitialized)
    {
        m_isInitialized = true;
        story->ScanVariable([this](std::shared_ptr<Variable> var) {
            if (var->GetUuid() == m_selectedVariableUuid)
            {
                m_selectedVariableName = var->GetVariableName();
            }
        });
    }

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Variable:");
    
    static ImGuiComboFlags flags = 0;
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);

    if (ImGui::BeginCombo("##variables", m_selectedVariableName.c_str(), flags))
    {
        int i = 0;
        story->ScanVariable([&i, this](std::shared_ptr<Variable> var) {
            const bool is_selected = (m_selectedIndex == i);
            std::string label = var->GetVariableName();
            
            if (ImGui::Selectable(label.c_str(), is_selected))
            {
                m_selectedIndex = i;
                m_selectedVariableName = label;
                m_variableNode->SetVariableUuid(var->GetUuid());
                m_variableNode->SetVariable(var);
                SetTitle(label);
            }

            // Set the initial focus when opening the combo
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }

            i++;
        });
        ImGui::EndCombo();
    }
    
    // Show variable details if selected
    if (!m_selectedVariableUuid.empty()) {
        ImGui::Separator();
        story->ScanVariable([this](std::shared_ptr<Variable> var) {
            if (var->GetUuid() == m_selectedVariableUuid) {
                ImGui::Text("Type: %s", Variable::ValueTypeToString(var->GetValueType()).c_str());
                ImGui::Text("Value: %s", var->GetValueAsString().c_str());
            }
        });
    }
}

void VariableNodeWidget::Draw()
{
    // Display variable name in the node
    ImGui::TextUnformatted(m_selectedVariableName.c_str());
}