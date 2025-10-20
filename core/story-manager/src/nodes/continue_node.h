#pragma once

#include <string>
#include "base_node.h"

class ContinueNode : public BaseNode
{
public:
    ContinueNode(const std::string &type = "continue-node");

    void Initialize() override;
};