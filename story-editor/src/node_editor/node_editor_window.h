#pragma once

#include <map>
#include <set>
#include <utility>

#include <imgui_node_editor.h>
#include "base_node_widget.h"
#include "window_base.h"
#include "i_story_manager.h"
#include "i_story_project.h"
#include "json.hpp"
#include "story_project.h"
#include "node_editor_page.h"
#include "nodes_factory.h"

namespace ed = ax::NodeEditor;

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

    NodeEditorWindow(IStoryManager &manager, NodesFactory &factory, IStoryProject::Type type = IStoryProject::Type::PROJECT_TYPE_STORY);
    ~NodeEditorWindow();
    virtual void Draw() override;

    void Initialize();
    void InitializeProject();
    void NewProject();
    void Clear();
    void Load(std::shared_ptr<StoryProject> story);
    void SaveNodePositions();
    void OpenFunction(const std::string &uuid, const std::string &name);

    std::shared_ptr<BaseNodeWidget> GetSelectedNode();
    std::shared_ptr<StoryProject> GetCurrentStory() { return m_story; }

private:
    IStoryManager &m_manager;
    NodesFactory &m_nodesFactory;
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

    void BuildNode(Node* node)
    {
        for (auto& input : node->Inputs)
        {
            input.Node = node;
            input.Kind = PinKind::Input;
        }

        for (auto& output : node->Outputs)
        {
            output.Node = node;
            output.Kind = PinKind::Output;
        }
    }

    template<class NodeType>
    struct Factory {
        static std::shared_ptr<BaseNodeWidget> create_func(IStoryManager &manager, std::shared_ptr<BaseNode> base) {
            return std::make_shared<NodeType>(manager, base);
        }
    };

    typedef std::shared_ptr<BaseNodeWidget> (*GenericCreator)(IStoryManager &manager, std::shared_ptr<BaseNode> base);
    typedef std::map<std::string, GenericCreator> Registry;
    Registry m_registry;

    template<class Derived>
    void registerNode(const std::string& key) {
        m_registry.insert(typename Registry::value_type(key, Factory<Derived>::create_func));
    }

    std::shared_ptr<BaseNodeWidget> CreateNodeWidget(const std::string& key, IStoryManager &manager, std::shared_ptr<BaseNode> base) {
        typename Registry::const_iterator i = m_registry.find(key);
        if (i == m_registry.end()) {
            throw std::invalid_argument(std::string(__PRETTY_FUNCTION__) +
                                        ": key not registered");
        }
        else
        {
            return i->second(manager, base);
        }
    }

    void LoadPage(const std::string &uuid, const std::string &name);
    ed::PinId GetInputPin(const std::string &modelNodeId, int pinIndex);
    ed::PinId GetOutputPin(const std::string &modelNodeId, int pinIndex);
    void CreateLink(std::shared_ptr<Connection> model, ed::PinId inId, ed::PinId outId);
    bool FillConnection(std::shared_ptr<Connection> c, ed::PinId pinId);
};

