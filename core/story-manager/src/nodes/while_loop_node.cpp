#include "while_loop_node.h"

WhileLoopNode::WhileLoopNode(const std::string &type)
    : BaseNode(type, "While Loop")
{
    SetBehavior(Behavior::BEHAVIOR_EXECUTION);
    
    // Port d'entrée 0: Exécution (EXECUTION_PORT)
    AddInputPort(Port::Type::EXECUTION_PORT, ">", true);
    
    // Port d'entrée 1: Condition (DATA_PORT)
    AddInputPort(Port::Type::DATA_PORT, "condition");
    
    // Ports de sortie pour l'exécution
    AddOutputPort(Port::Type::EXECUTION_PORT, "body", true);  // Port 0
    AddOutputPort(Port::Type::EXECUTION_PORT, "done", true);  // Port 1
    
    // Initialiser les données internes
    nlohmann::json j;
    j["node_type"] = "while_loop";
    SetInternalData(j);
}

void WhileLoopNode::Initialize()
{
    // Charger les données sauvegardées si nécessaire
    nlohmann::json j = GetInternalData();
}
