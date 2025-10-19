// story-editor/src/node_editor/send_signal_node_widget.h
#pragma once

#include "base_node_widget.h"
#include "i_story_manager.h"
#include "i_story_project.h"
#include "send_signal_node.h"

class SendSignalNodeWidget : public BaseNodeWidget
{
public:
    SendSignalNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node);

    void Draw() override;
    void DrawProperties(std::shared_ptr<IStoryProject> story) override;
    void Initialize() override;

private:
    std::shared_ptr<SendSignalNode> m_sendSignalNode;
};