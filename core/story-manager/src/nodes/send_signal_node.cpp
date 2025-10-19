// core/story-manager/src/nodes/send_signal_node.cpp
#include "send_signal_node.h"

SendSignalNode::SendSignalNode(const std::string &type)
    : BaseNode(type, "Send Signal")
{
    SetBehavior(BaseNode::BEHAVIOR_EXECUTION);
    SetupExecutionPorts(true, 1, true);  // IN 0, OUT 0
    
    AddInputPort(Port::Type::DATA_PORT, "Signal ID");
    
    nlohmann::json j{{"signal_id", m_signalId}};
    SetInternalData(j);
}

void SendSignalNode::Initialize()
{
    nlohmann::json j = GetInternalData();
    if (j.contains("signal_id")) {
        m_signalId = j["signal_id"].get<uint32_t>();
    }
}

void SendSignalNode::SetSignalId(uint32_t id)
{
    m_signalId = id;
    nlohmann::json j{{"signal_id", m_signalId}};
    SetInternalData(j);
}