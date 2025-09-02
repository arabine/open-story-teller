#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <set>

#include "base_node_widget.h"
#include "i_story_manager.h"
#include "i_story_project.h"
#include "gui.h"
#include "print_node.h"
#include "media_node.h"

class PrintNodeWidget : public BaseNodeWidget
{
public:

    static const int MAX_PRINT_SIZE = 128;
    PrintNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node);

    void Draw() override;

    virtual void DrawProperties(std::shared_ptr<IStoryProject> story) override;
    virtual void Initialize() override;

private:
    IStoryManager &m_manager;
    std::shared_ptr<PrintNode> m_printNode;

    static char m_buffer[MAX_PRINT_SIZE];
};
