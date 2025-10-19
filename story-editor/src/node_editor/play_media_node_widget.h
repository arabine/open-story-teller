// story-editor/src/node_editor/play_media_node_widget.h
#pragma once

#include "base_node_widget.h"
#include "i_story_manager.h"
#include "i_story_project.h"
#include "play_media_node.h"

class PlayMediaNodeWidget : public BaseNodeWidget
{
public:
    PlayMediaNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node);

    void Draw() override;
    void DrawProperties(std::shared_ptr<IStoryProject> story) override;
    void Initialize() override;

private:
    std::shared_ptr<PlayMediaNode> m_playMediaNode;
    
};