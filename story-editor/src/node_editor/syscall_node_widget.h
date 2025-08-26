#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <set>

#include "base_node_widget.h"
#include "i_story_manager.h"
#include "i_story_project.h"
#include "gui.h"
#include "syscall_node.h"
#include <imgui_node_editor.h>
#include "media_node.h"

class SyscallNodeWidget : public BaseNodeWidget
{
public:
    SyscallNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node);

    void Draw() override;

    virtual void DrawProperties(std::shared_ptr<IStoryProject> story) override;
    virtual void Initialize() override;

private:
    std::shared_ptr<SyscallNode> m_syscallNode;
};
