// story-editor/src/node_editor/wait_event_node_widget.h
#pragma once

#include "base_node_widget.h"
#include "i_story_manager.h"
#include "i_story_project.h"
#include "wait_event_node.h"

class WaitEventNodeWidget : public BaseNodeWidget
{
public:
    WaitEventNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node);

    void Draw() override;
    void DrawProperties(std::shared_ptr<IStoryProject> story) override;
    void Initialize() override;

private:
    std::shared_ptr<WaitEventNode> m_waitEventNode;
    
    void DrawEventMaskEditor();
    std::string GetEventMaskDescription(uint32_t mask);
};