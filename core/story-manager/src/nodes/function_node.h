#pragma once

#include <string>
#include "i_story_manager.h"
#include "base_node.h"
#include "i_script_node.h"
#include "i_story_project.h"

class FunctionNode : public BaseNode
{
public:
    FunctionNode(const std::string &type);

    virtual void Initialize() override;

private:

};

