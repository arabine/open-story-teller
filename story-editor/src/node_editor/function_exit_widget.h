#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <set>

#include "base_node_widget.h"
#include "i_story_manager.h"
#include "i_story_project.h"
#include "function_exit_node.h"
#include "gui.h"


class FunctionExitWidget : public BaseNodeWidget
{
public:
    FunctionExitWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
        : BaseNodeWidget(manager, node)
        , m_manager(manager)
    {
        m_functionExitNode = std::dynamic_pointer_cast<FunctionExitNode>(node);
        SetTitle("Function Exit");
    }

    void Draw() override {
        ImGui::SetNextItemWidth(100.f);
    }

    virtual void DrawProperties(std::shared_ptr<IStoryProject> story) override {

    }
    virtual void Initialize() override {

    }

private:
    IStoryManager &m_manager;

    std::shared_ptr<FunctionExitNode> m_functionExitNode;

};
