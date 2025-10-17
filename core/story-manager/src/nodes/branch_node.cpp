#include "branch_node.h"
#include "story_project.h"
#include "connection.h"
#include "sys_lib.h"

BranchNode::BranchNode(const std::string &type)
    : BaseNode(type, "Branch Node")
{
    // Configuration du nœud de branchement
    SetBehavior(BaseNode::BEHAVIOR_EXECUTION);
    
    // Port d'entrée 0: Exécution (EXECUTION_PORT)
    // Port d'entrée 1: Condition (DATA_PORT)
    AddInputPort(Port::EXECUTION_PORT, ">", true);
    AddInputPort(Port::DATA_PORT, "condition");
    
    // Port de sortie 0: Branche TRUE (EXECUTION_PORT)
    // Port de sortie 1: Branche FALSE (EXECUTION_PORT)
    AddOutputPort(Port::EXECUTION_PORT, "true", true);
    AddOutputPort(Port::EXECUTION_PORT, "false", true);
    
    // Initialiser les données internes
    nlohmann::json j;
    j["node_type"] = "branch";
    j["version"] = 1;
    SetInternalData(j);
}

void BranchNode::Initialize()
{
    // Charger les données sauvegardées
    nlohmann::json j = GetInternalData();
    
    // Vérifier la version pour compatibilité future
    if (j.contains("version"))
    {
        int version = j["version"].get<int>();
        // Gérer les migrations de version si nécessaire
    }
}