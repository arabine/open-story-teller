#pragma once

#include "base_node_widget.h"
#include "continue_node.h"

class ContinueNodeWidget : public BaseNodeWidget
{
public:
    ContinueNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> base);
    virtual ~ContinueNodeWidget() = default;

    void Draw() override;
    void DrawProperties(std::shared_ptr<IStoryProject> story) override;

private:
    std::shared_ptr<ContinueNode> m_continueNode;
};
