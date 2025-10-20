#pragma once

#include <string>
#include "base_node.h"

class WhileLoopNode : public BaseNode
{
public:
    WhileLoopNode(const std::string &type = "while-loop-node");

    void Initialize() override;
};
