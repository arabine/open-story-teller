#pragma once

#include <string>

#include "variable.h"
#include "i_story_manager.h"
#include "base_node.h"
#include "i_script_node.h"
#include "i_story_project.h"

class SyscallNode : public BaseNode
{
public:

    SyscallNode(const std::string &type = "syscall-node");

    virtual void Initialize() override;

private:


};

