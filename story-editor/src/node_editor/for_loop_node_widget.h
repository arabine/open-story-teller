// story-editor/src/node_editor/for_loop_node_widget.h
#pragma once

#include "base_node_widget.h"
#include "for_loop_node.h"

class ForLoopNodeWidget : public BaseNodeWidget
{
public:
    ForLoopNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> base);
    virtual ~ForLoopNodeWidget() = default;

    void Draw() override;
    void DrawProperties(std::shared_ptr<IStoryProject> story) override;

private:
    std::shared_ptr<ForLoopNode> m_forLoopNode;
};