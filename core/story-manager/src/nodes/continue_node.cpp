#include "continue_node.h"

ContinueNode::ContinueNode(const std::string &type)
    : BaseNode(type, "Continue")
{
    SetBehavior(Behavior::BEHAVIOR_EXECUTION);
    
    // Port d'entrée 0: Exécution (EXECUTION_PORT)
    AddInputPort(Port::Type::EXECUTION_PORT, ">", true);
    
    // Pas de port de sortie : le flux est redirigé vers le début de la boucle
    // par le générateur TAC
    
    // Initialiser les données internes
    nlohmann::json j;
    j["node_type"] = "continue";
    SetInternalData(j);
}

void ContinueNode::Initialize()
{
    // Charger les données sauvegardées si nécessaire
    nlohmann::json j = GetInternalData();
}