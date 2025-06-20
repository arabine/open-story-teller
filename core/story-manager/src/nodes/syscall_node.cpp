#include "syscall_node.h"
#include "story_project.h"
#include "connection.h"
#include "sys_lib.h"


SyscallNode::SyscallNode(const std::string &type)
    : BaseNode(type, "Syscall Node")
{
    nlohmann::json j{ {"syscall_number", m_syscallNumber} };
    SetInternalData(j);
}

void SyscallNode::Initialize()
{
    nlohmann::json j = GetInternalData();

    m_syscallNumber = j["syscall_number"].get<int>();
}

int SyscallNode::GetArgumentsSize() const
{
    int size = 0;

    switch (m_syscallNumber)
    {
    case SYSCALL_PLAY_MEDIA:
        size = 2;
        break;    
    default:
        break;
    }


    return size;
}


