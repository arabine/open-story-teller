#pragma once

#include <string>
#include "i_story_manager.h"
#include "base_node.h"

class IScriptNode
{
public:
    virtual ~IScriptNode() {}

    virtual std::string Build(IStoryManager &story, int nb_out_conns, BaseNode &base) = 0;
    virtual std::string GenerateConstants(IStoryManager &story, int nb_out_conns, BaseNode &base) = 0;
};
