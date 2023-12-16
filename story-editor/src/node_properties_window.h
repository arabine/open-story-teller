#pragma once

#include "window_base.h"
#include "gui.h"

#include "base_node.h"

class NodePropertiesWindow : public WindowBase
{
public:
    NodePropertiesWindow();

    void Initialize();
    virtual void Draw() override;

    void SetSelectedNode(std::shared_ptr<BaseNode> node);

private:
    std::shared_ptr<BaseNode> m_selectedNode;

};

