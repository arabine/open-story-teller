#include "for_loop_node.h"

ForLoopNode::ForLoopNode(const std::string &type)
    : BaseNode(type, "For Loop")
{
    SetBehavior(Behavior::BEHAVIOR_EXECUTION);
    
    // Port d'entrée 0: Exécution (EXECUTION_PORT)
    AddInputPort(Port::Type::EXECUTION_PORT, ">", true);
    
    // Ports d'entrée pour les données
    AddInputPort(Port::Type::DATA_PORT, "start");   // Port 1
    AddInputPort(Port::Type::DATA_PORT, "end");     // Port 2
    AddInputPort(Port::Type::DATA_PORT, "step");    // Port 3
    
    // Ports de sortie pour l'exécution
    AddOutputPort(Port::Type::EXECUTION_PORT, "body", true);  // Port 0
    AddOutputPort(Port::Type::EXECUTION_PORT, "done", true);  // Port 1
    
    // Port de sortie pour l'index courant
    AddOutputPort(Port::Type::DATA_PORT, "index");  // Port 2
    
    // Initialiser les données internes
    nlohmann::json j;
    j["start_index"] = m_startIndex;
    j["end_index"] = m_endIndex;
    j["step"] = m_step;
    j["node_type"] = "for_loop";
    SetInternalData(j);
}

void ForLoopNode::Initialize()
{
    // Charger les données sauvegardées
    nlohmann::json j = GetInternalData();
    
    if (j.contains("start_index")) {
        m_startIndex = j["start_index"].get<int>();
    }
    if (j.contains("end_index")) {
        m_endIndex = j["end_index"].get<int>();
    }
    if (j.contains("step")) {
        m_step = j["step"].get<int>();
    }
}
