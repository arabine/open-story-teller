#pragma once

#include <map>
#include <set>
#include <utility>

#include "base_node_widget.h"
#include "window_base.h"
#include "i_story_manager.h"
#include "i_story_project.h"
#include "json.hpp"
#include "story_project.h"
#include "node_editor_page.h"
#include "nodes_factory.h"
#include "node_widget_factory.h"
#include "ImNodeFlow.h"


# ifdef _MSC_VER
# define portable_strcpy    strcpy_s
# define portable_sprintf   sprintf_s
# else
# define portable_strcpy    strcpy
# define portable_sprintf   sprintf
# endif

class NodeEditorWindow  : public WindowBase
{
public:

    NodeEditorWindow(IStoryManager &manager, NodesFactory &factory, NodeWidgetFactory &widgetFactory, IStoryProject::Type type = IStoryProject::Type::PROJECT_TYPE_STORY);
    ~NodeEditorWindow();
    virtual void Draw() override;

    void Initialize();
    void InitializeProject();
    void NewProject();
    void Clear();
    void Load(std::shared_ptr<StoryProject> story);
    void SaveNodePositions();
    void OpenFunction(const std::string &uuid, const std::string &name);
    void SaveNodesToProject();

    std::shared_ptr<BaseNodeWidget> GetSelectedNode();
    std::shared_ptr<StoryProject> GetCurrentStory() { return m_story; }

private:
    IStoryManager &m_manager;
    NodesFactory &m_nodesFactory;
    NodeWidgetFactory &m_widgetFactory;
    IStoryProject::Type m_editorType{IStoryProject::Type::PROJECT_TYPE_STORY};
    bool m_loaded{false};

    // "MainUuid" is the entry point editor context. You always need to create one.
    // Then each function can have its own editor context, for example if you want to create multiple graphs.
    // the key is main, or the UUID of the function
    std::list<std::shared_ptr<NodeEditorPage>> m_pages;
    std::shared_ptr<NodeEditorPage> m_currentPage;
    std::string m_newPageUuid;
    std::string m_newPageName;
    std::shared_ptr<StoryProject> m_story;
    std::list<std::shared_ptr<NodeEditorPage>> m_callStack;
   
    void ToolbarUI();


    void LoadPage(const std::string &uuid, const std::string &name);
};

