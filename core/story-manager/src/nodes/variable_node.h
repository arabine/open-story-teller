#pragma once

#include <string>

#include "variable.h"
#include "i_story_manager.h"
#include "base_node.h"
#include "i_script_node.h"
#include "i_story_project.h"

class VariableNode : public BaseNode
{
public:

    VariableNode(const std::string &type = "variable-node");

    virtual void Initialize() override;

    void SetVariableUuid(const std::string &uuid);

    std::string GetVariableUuid() const;

private:
    std::string m_variableUuid;

};

