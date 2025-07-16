#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <set>

#include "base_node_widget.h"
#include "i_story_manager.h"
#include "i_story_project.h"
#include "call_function_node.h"
#include "gui.h"
#include <imgui_node_editor.h>


class CallFunctionNodeWidget : public BaseNodeWidget
{
public:
    CallFunctionNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node);

    void Draw() override;

    virtual void DrawProperties() override;
    virtual void Initialize() override;

private:
    IStoryManager &m_manager;

    std::shared_ptr<CallFunctionNode> m_callFunctionNode;

    std::string m_functionName;
    std::string m_functionUuid; 
};
