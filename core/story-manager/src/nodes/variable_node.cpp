#include "variable_node.h"
#include "story_project.h"
#include "connection.h"
#include "sys_lib.h"

VariableNode::VariableNode(const std::string &type)
    : BaseNode(type, "Variable Node", BaseNode::BEHAVIOR_DATA)
{
    nlohmann::json j{ {"uuid", ""} };
    SetInternalData(j);
    
    // Add data output port
    AddOutputPort(Port::DATA_PORT, "value");
}

void VariableNode::Initialize()
{
    nlohmann::json j = GetInternalData();
    m_variableUuid = j["uuid"].get<std::string>();
}

void VariableNode::SetVariableUuid(const std::string &uuid)
{
    m_variableUuid = uuid;

    nlohmann::json j;
    j["uuid"] = m_variableUuid;
    SetInternalData(j);
}

std::string VariableNode::GetVariableUuid() const
{
    return m_variableUuid;
}

void VariableNode::SetVariable(std::shared_ptr<Variable> var)
{
    m_variable = var;
    if (var) {
        SetVariableUuid(var->GetUuid());
        SetTitle(var->GetLabel());
    }
}

std::shared_ptr<Variable> VariableNode::GetVariable() const
{
    return m_variable;
}