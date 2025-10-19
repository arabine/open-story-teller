// story-editor/src/node_editor/wait_delay_node_widget.h
#pragma once

#include "base_node_widget.h"
#include "i_story_manager.h"
#include "i_story_project.h"
#include "wait_delay_node.h"

class WaitDelayNodeWidget : public BaseNodeWidget
{
public:
    WaitDelayNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node);

    void Draw() override;
    void DrawProperties(std::shared_ptr<IStoryProject> story) override;
    void Initialize() override;

private:
    std::shared_ptr<WaitDelayNode> m_waitDelayNode;
};