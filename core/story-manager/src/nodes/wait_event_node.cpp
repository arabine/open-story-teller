// core/story-manager/src/nodes/wait_event_node.cpp
#include "wait_event_node.h"

WaitEventNode::WaitEventNode(const std::string &type)
    : BaseNode(type, "Wait Event")
{
    SetBehavior(BaseNode::BEHAVIOR_EXECUTION);
    SetupExecutionPorts(true, 1, true);  // IN 0 (exec), OUT 0 (exec)
    
    AddInputPort(Port::Type::DATA_PORT, "Event Mask");
    AddInputPort(Port::Type::DATA_PORT, "Timeout");
    
    nlohmann::json j{
        {"event_mask", m_eventMask},
        {"timeout", m_timeout},
        {"result_variable_uuid", m_resultVariableUuid}
    };
    SetInternalData(j);
}

void WaitEventNode::Initialize()
{
    nlohmann::json j = GetInternalData();
    
    if (j.contains("event_mask")) {
        m_eventMask = j["event_mask"].get<uint32_t>();
    }
    if (j.contains("timeout")) {
        m_timeout = j["timeout"].get<uint32_t>();
    }
    if (j.contains("result_variable_uuid")) {
        m_resultVariableUuid = j["result_variable_uuid"].get<std::string>();
    }
}

void WaitEventNode::SetEventMask(uint32_t mask)
{
    m_eventMask = mask;
    nlohmann::json j = GetInternalData();
    j["event_mask"] = m_eventMask;
    SetInternalData(j);
}

void WaitEventNode::SetTimeout(uint32_t ms)
{
    m_timeout = ms;
    nlohmann::json j = GetInternalData();
    j["timeout"] = m_timeout;
    SetInternalData(j);
}

void WaitEventNode::SetResultVariable(const std::string& varUuid)
{
    m_resultVariableUuid = varUuid;
    nlohmann::json j = GetInternalData();
    j["result_variable_uuid"] = m_resultVariableUuid;
    SetInternalData(j);
}

std::shared_ptr<Variable> WaitEventNode::ResolveResultVariable(
    const std::vector<std::shared_ptr<Variable>>& variables) const
{
    if (m_resultVariableUuid.empty()) {
        return nullptr;
    }
    
    for (const auto& var : variables) {
        if (var->GetUuid() == m_resultVariableUuid) {
            return var;
        }
    }
    
    return nullptr;
}