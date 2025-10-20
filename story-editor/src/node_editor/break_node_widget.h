#pragma once

#include "base_node_widget.h"
#include "break_node.h"

class BreakNodeWidget : public BaseNodeWidget
{
public:
    BreakNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> base);
    virtual ~BreakNodeWidget() = default;

    void Draw() override;
    void DrawProperties(std::shared_ptr<IStoryProject> story) override;

private:
    std::shared_ptr<BreakNode> m_breakNode;
};
