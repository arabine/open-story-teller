#include "syscall_node.h"
#include "story_project.h"
#include "connection.h"
#include "sys_lib.h"


SyscallNode::SyscallNode(const std::string &type)
    : BaseNode(type, "Syscall Node")
{
    nlohmann::json j{ {"uuid", ""} };
    SetInternalData(j);
}

void SyscallNode::Initialize()
{
    nlohmann::json j = GetInternalData();

    // m_variableUuid = j["uuid"].get<std::string>();
}



