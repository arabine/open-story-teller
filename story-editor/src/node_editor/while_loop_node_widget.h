// story-editor/src/node_editor/while_loop_node_widget.h
#pragma once

#include "base_node_widget.h"
#include "while_loop_node.h"

class WhileLoopNodeWidget : public BaseNodeWidget
{
public:
    WhileLoopNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> base);
    virtual ~WhileLoopNodeWidget() = default;

    void Draw() override;
    void DrawProperties(std::shared_ptr<IStoryProject> story) override;

private:
    std::shared_ptr<WhileLoopNode> m_whileLoopNode;
};