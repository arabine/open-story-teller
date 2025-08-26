#pragma once

#include "window_base.h"
#include "gui.h"

#include "base_node_widget.h"

class PropertiesWindow : public WindowBase
{
public:
    PropertiesWindow();

    void Initialize();
    void Draw() override;
    void Draw(std::shared_ptr<IStoryProject> story);

    void SetSelectedNode(std::shared_ptr<BaseNodeWidget> node);

private:
    std::shared_ptr<BaseNodeWidget> m_selectedNode;

};

