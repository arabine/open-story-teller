#pragma once

#include <string>
#include "i_story_manager.h"
#include "base_node.h"
#include "i_script_node.h"
#include "i_story_project.h"
#include "variable.h"

class VariableNode : public BaseNode
{
public:
    VariableNode(const std::string &type);

    virtual void Initialize() override;

    void SetVariableUuid(const std::string &uuid);
    std::string GetVariableUuid() const;
    
    void SetVariable(std::shared_ptr<Variable> var);
    std::shared_ptr<Variable> GetVariable() const;

    bool ResolveVariable(const std::vector<std::shared_ptr<Variable>>& variables);

private:
    std::string m_variableUuid;
    std::shared_ptr<Variable> m_variable;
};