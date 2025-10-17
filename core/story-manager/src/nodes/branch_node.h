#pragma once

#include <string>
#include "i_story_manager.h"
#include "base_node.h"
#include "i_script_node.h"
#include "i_story_project.h"

/**
 * @brief Nœud de branchement conditionnel simple
 * 
 * Ce nœud évalue une condition booléenne et exécute l'une des deux branches.
 * La condition provient d'un OperatorNode ou de toute autre source de données.
 * 
 * Convention booléenne:
 * - 0 = false
 * - toute autre valeur = true
 * 
 * Ports:
 * - Entrée 0: Flux d'exécution (EXECUTION_PORT)
 * - Entrée 1: Condition (DATA_PORT) - résultat d'un OperatorNode par exemple
 * - Sortie 0: Branche TRUE (EXECUTION_PORT)
 * - Sortie 1: Branche FALSE (EXECUTION_PORT)
 * 
 * Utilisation typique:
 * [Variable A] ──┐
 *                ├──> [OperatorNode >] ──> [BranchNode]
 * [Variable B] ──┘                            ↓      ↓
 *                                          TRUE    FALSE
 */
class BranchNode : public BaseNode
{
public:
    BranchNode(const std::string &type);

    virtual void Initialize() override;
};