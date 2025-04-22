#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <set>

#include "base_node_widget.h"
#include "i_story_manager.h"
#include "i_story_project.h"
#include "gui.h"
#include <imgui_node_editor.h>


class FunctionNodeWidget : public BaseNodeWidget
{
public:
    FunctionNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node);

    void Draw() override;

    virtual void DrawProperties() override;
    virtual void Initialize() override;

private:
    IStoryManager &m_manager;
    std::string m_functionName;
    std::string m_functionUuid; 
};
