#pragma once

#include <vector>
#include <map>
#include <memory>

#include "base_node_widget.h"
#include "i_story_manager.h"
#include "i_story_project.h"
#include "gui.h"
#include "branch_node.h"

/**
 * @brief Widget pour le nœud Branch (If/Else)
 * 
 * Gère l'affichage et les propriétés d'un nœud de branchement conditionnel simple.
 * La condition est évaluée selon la convention: 0 = false, toute autre valeur = true
 * 
 * Ports:
 * - Entrée 0: Flux d'exécution
 * - Entrée 1: Condition booléenne (provient généralement d'un OperatorNode)
 * - Sortie 0: Branche TRUE (condition != 0)
 * - Sortie 1: Branche FALSE (condition == 0)
 */
class BranchNodeWidget : public BaseNodeWidget
{
public:
    BranchNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node);

    void Draw() override;
    void DrawProperties(std::shared_ptr<IStoryProject> story) override;
    void Initialize() override;

private:
    std::shared_ptr<BranchNode> m_branchNode;
};