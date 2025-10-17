#include "branch_node_widget.h"

#include <sstream>
#include "IconsMaterialDesignIcons.h"
#include "story_project.h"

BranchNodeWidget::BranchNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
    : BaseNodeWidget(manager, node)
{
    m_branchNode = std::dynamic_pointer_cast<BranchNode>(node);
}

void BranchNodeWidget::Initialize()
{
    BaseNodeWidget::Initialize();
}

void BranchNodeWidget::DrawProperties(std::shared_ptr<IStoryProject> story)
{
    ImGui::AlignTextToFramePadding();
    
    // Informations sur le nœud de branchement
    ImGui::SeparatorText("Branch Node Properties");
    
    ImGui::TextWrapped("This node evaluates a boolean condition and executes "
                      "one of two paths based on the result.");
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}

void BranchNodeWidget::Draw()
{
    // Icône et texte du nœud
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
    ImGui::Text(ICON_MDI_SOURCE_BRANCH " Branch");
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    
    // Affichage de la condition
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "if (condition)");
    
    ImGui::Spacing();
    
    // Affichage visuel des branches
    ImGui::BeginGroup();
    {
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), ICON_MDI_CHECK " TRUE");
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), ICON_MDI_CLOSE " FALSE");
    }
    ImGui::EndGroup();
}