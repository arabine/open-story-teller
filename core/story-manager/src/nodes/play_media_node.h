// core/story-manager/src/nodes/play_media_node.h
#pragma once

#include <string>
#include "base_node.h"
#include "i_story_project.h"

class PlayMediaNode : public BaseNode
{
public:
    PlayMediaNode(const std::string &type = "play-media-node");

    void Initialize() override;
    
    // Variable optionnelle pour le status de retour
    std::string GetStatusVariableUuid() const { return m_statusVariableUuid; }
    void SetStatusVariable(const std::string& varUuid);

private:
    std::string m_statusVariableUuid;  // Optionnel
};