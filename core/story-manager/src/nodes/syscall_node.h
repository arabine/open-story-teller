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

    enum SysCallType  : int  {
        SYSCALL_PLAY_MEDIA = 1,
        SYSCALL_WAIT_EVENT = 2,
        SYSCALL_SEND_SIGNAL = 3,
        SYSCALL_PRINT = 4,
        SYSCALL_WAIT_MILLISEC = 5
    };

    SyscallNode(const std::string &type = "syscall-node");

    virtual void Initialize() override;

    int GetSyscallNumber() const  {
        return m_syscallNumber;
    }

    void SetSyscallNumber(int syscallNumber) {
        m_syscallNumber = syscallNumber;
        SetInternalData({{"syscall_number", m_syscallNumber}});
    }

    int GetArgumentsSize() const;

private:
    int m_syscallNumber{4};

};

