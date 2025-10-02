#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <set>

#include "base_node_widget.h"
#include "i_story_manager.h"
#include "i_story_project.h"
#include "function_entry_node.h"
#include "gui.h"

class FunctionEntryWidget : public BaseNodeWidget
{
public:
    FunctionEntryWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
        : BaseNodeWidget(manager, node)
        , m_manager(manager)
    {
        m_functionEntryNode = std::dynamic_pointer_cast<FunctionEntryNode>(node);
        SetTitle("Function Entry");
    }

    void Draw() override {
        ImGui::SetNextItemWidth(100.f);
    }

    virtual bool HasSync() const override { 
        return false;
    }

    virtual void DrawProperties(std::shared_ptr<IStoryProject> story) override {

    }
    virtual void Initialize() override {

    }

private:
    IStoryManager &m_manager;

    std::shared_ptr<FunctionEntryNode> m_functionEntryNode;
 
};
