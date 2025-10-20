#include "break_node.h"

BreakNode::BreakNode(const std::string &type)
    : BaseNode(type, "Break")
{
    SetBehavior(Behavior::BEHAVIOR_EXECUTION);
    
    // Port d'entrée 0: Exécution (EXECUTION_PORT)
    AddInputPort(Port::Type::EXECUTION_PORT, ">", true);
    
    // Pas de port de sortie : le flux est redirigé vers la fin de la boucle
    // par le générateur TAC
    
    // Initialiser les données internes
    nlohmann::json j;
    j["node_type"] = "break";
    SetInternalData(j);
}

void BreakNode::Initialize()
{
    // Charger les données sauvegardées si nécessaire
    nlohmann::json j = GetInternalData();
}
