// core/story-manager/src/nodes/wait_delay_node.cpp
#include "wait_delay_node.h"

WaitDelayNode::WaitDelayNode(const std::string &type)
    : BaseNode(type, "Wait Delay")
{
    SetBehavior(BaseNode::BEHAVIOR_EXECUTION);
    SetupExecutionPorts(true, 1, true);  // IN 0, OUT 0
    
    AddInputPort(Port::Type::DATA_PORT, "Duration");
    
    nlohmann::json j{{"duration_ms", m_durationMs}};
    SetInternalData(j);
}

void WaitDelayNode::Initialize()
{
    nlohmann::json j = GetInternalData();
    if (j.contains("duration_ms")) {
        m_durationMs = j["duration_ms"].get<uint32_t>();
    }
}

void WaitDelayNode::SetDuration(uint32_t ms)
{
    m_durationMs = ms;
    nlohmann::json j{{"duration_ms", m_durationMs}};
    SetInternalData(j);
}