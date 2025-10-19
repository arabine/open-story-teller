// core/story-manager/src/nodes/wait_delay_node.h
#pragma once

#include <string>
#include "base_node.h"
#include "i_story_project.h"

class WaitDelayNode : public BaseNode
{
public:
    WaitDelayNode(const std::string &type = "wait-delay-node");

    void Initialize() override;

    // Durée en millisecondes
    uint32_t GetDuration() const { return m_durationMs; }
    void SetDuration(uint32_t ms);

private:
    uint32_t m_durationMs{1000};
};