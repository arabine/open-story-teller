// core/story-manager/src/nodes/play_media_node.cpp
#include "play_media_node.h"

PlayMediaNode::PlayMediaNode(const std::string &type)
    : BaseNode(type, "Play Media")
{
    SetBehavior(BaseNode::BEHAVIOR_EXECUTION);
    SetupExecutionPorts(true, 1, true);  // IN 0 (exec), OUT 0 (exec)
    
    // Ports data
    AddInputPort(Port::Type::DATA_PORT, "Image");
    AddInputPort(Port::Type::DATA_PORT, "Sound");
    
    nlohmann::json j{
        {"status_variable_uuid", m_statusVariableUuid}
    };
    SetInternalData(j);
}

void PlayMediaNode::Initialize()
{
    nlohmann::json j = GetInternalData();
    
    if (j.contains("status_variable_uuid")) {
        m_statusVariableUuid = j["status_variable_uuid"].get<std::string>();
    }
}

void PlayMediaNode::SetStatusVariable(const std::string& varUuid)
{
    m_statusVariableUuid = varUuid;
    nlohmann::json j = GetInternalData();
    j["status_variable_uuid"] = m_statusVariableUuid;
    SetInternalData(j);
}