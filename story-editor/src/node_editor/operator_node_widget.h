#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <set>

#include "base_node_widget.h"
#include "i_story_manager.h"
#include "i_story_project.h"
#include "gui.h"
#include "operator_node.h"
#include <imgui_node_editor.h>
#include "media_node.h"

class OperatorNodeWidget : public BaseNodeWidget
{
public:
    OperatorNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node);

    void Draw() override;

    virtual void DrawProperties(std::shared_ptr<IStoryProject> story) override;
    virtual void Initialize() override;

private:
    IStoryManager &m_manager;
    std::shared_ptr<OperatorNode> m_opNode;
    int m_selectedIndex{-1};
    OperatorNode::OperationType m_selectedOperatorType{OperatorNode::OperationType::ADD};
    std::string m_selectedOperatorSymbol{"+"};
};
