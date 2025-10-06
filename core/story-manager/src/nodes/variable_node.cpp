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

bool VariableNode::ResolveVariable(const std::vector<std::shared_ptr<Variable>>& variables)
{
    // Si la variable est déjà résolue, pas besoin de chercher
    if (m_variable) {
        return true;
    }
    
    // Si pas d'UUID, impossible de résoudre
    if (m_variableUuid.empty()) {
        std::cout << "WARNING: VariableNode " << GetId() 
                  << " has no variable UUID!" << std::endl;
        return false;
    }
    
    // Chercher la variable correspondant à l'UUID
    for (const auto& var : variables) {
        if (var->GetUuid() == m_variableUuid) {
            m_variable = var;
            std::cout << "✓ Resolved variable '" << var->GetVariableName() 
                      << "' for VariableNode " << GetId() << std::endl;
            return true;
        }
    }
    
    std::cout << "ERROR: Could not resolve variable UUID " << m_variableUuid 
              << " for VariableNode " << GetId() << std::endl;
    return false;
}
