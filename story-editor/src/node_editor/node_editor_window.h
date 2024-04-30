#pragma once

#include <map>
#include <set>
#include <set>

#include <imgui_node_editor.h>
#include "base_node_widget.h"
#include "window_base.h"
#include "i_story_manager.h"
#include "json.hpp"
#include "story_project.h"


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
    struct EditorLink {
        ed::LinkId Id;
        ed::PinId  InputId;
        ed::PinId  OutputId;
    };

    // Stuff from ImGuiNodeEditor, each element has a unique ID within one editor
    struct LinkInfo
    {

        LinkInfo()
        {
            ed_link = std::make_shared<EditorLink>();
            model = std::make_shared<Connection>();
        }

        std::shared_ptr<EditorLink> ed_link;
        std::shared_ptr<Connection> model;
    };

    NodeEditorWindow(IStoryManager &manager);
    ~NodeEditorWindow();
    virtual void Draw() override;

    void Initialize();
    void Clear();
    void Load(std::shared_ptr<StoryProject> story);

    std::shared_ptr<BaseNodeWidget> GetSelectedNode();

private:
    IStoryManager &m_manager;

    bool m_loaded{false};

    ed::EditorContext* m_context = nullptr;

    std::shared_ptr<StoryProject> m_story;
    std::list<std::shared_ptr<BaseNodeWidget>>   m_nodes;
    std::list<std::shared_ptr<LinkInfo>>   m_links;                // List of live links. It is dynamic unless you want to create read-only view over nodes.
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

    ed::PinId GetInputPin(const std::string &modelNodeId, int pinIndex);
    ed::PinId GetOutputPin(const std::string &modelNodeId, int pinIndex);
    void CreateLink(std::shared_ptr<Connection> model, ed::PinId inId, ed::PinId outId);
    std::shared_ptr<Connection> LinkToModel(ed::PinId InputId, ed::PinId OutputId);
};

