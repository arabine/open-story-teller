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
    virtual std::string Build(IStoryPage &page, const StoryOptions &options, int nb_out_conns) override;

    virtual std::string GenerateAssembly() const { return ""; }

    void StoreInternalData();

    void SetVariableUuid(const std::string &uuid) {
        m_variableUuid = uuid;
    }

    std::string GetVariableUuid() const {
        return m_variableUuid;
    }

private:
    std::string m_variableUuid;

};

