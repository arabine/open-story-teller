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

    void StoreInternalData();

    void SetVariable(const std::shared_ptr<Variable> v) {
        m_variable = v;
    }

    std::shared_ptr<Variable> GetVariable() const {
        return m_variable;
    }

private:
std::shared_ptr<Variable> m_variable;

};

