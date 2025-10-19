// core/story-manager/src/nodes/send_signal_node.h
#pragma once

#include <string>
#include "base_node.h"
#include "i_story_project.h"

class SendSignalNode : public BaseNode
{
public:
    SendSignalNode(const std::string &type = "send-signal-node");

    void Initialize() override;

    // ID du signal à envoyer
    uint32_t GetSignalId() const { return m_signalId; }
    void SetSignalId(uint32_t id);

private:
    uint32_t m_signalId{0};
};