#pragma once

#include <string>
#include "base_node.h"

class BreakNode : public BaseNode
{
public:
    BreakNode(const std::string &type = "break-node");

    void Initialize() override;
};
