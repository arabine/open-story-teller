#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <set>

#include "base_node_widget.h"
#include "i_story_manager.h"
#include "i_story_project.h"
#include "gui.h"
#include "variable_node.h"
#include <imgui_node_editor.h>
#include "media_node.h"

class VariableNodeWidget : public BaseNodeWidget
{
public:
    VariableNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node);

    void Draw() override;

    virtual void DrawProperties() override;
    virtual void Initialize() override;

private:
    IStoryManager &m_manager;
    std::shared_ptr<VariableNode> m_variableNode;
    int m_selectedIndex{-1};
    std::string m_selectedVariable;
};
