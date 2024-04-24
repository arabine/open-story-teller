#pragma once

#include <map>
#include <set>
#include <set>

#include <imgui_node_editor.h>
#include "base_node_widget.h"
#include "window_base.h"
#include "i_story_manager.h"
#include "json.hpp"


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

    NodeEditorWindow(IStoryManager &proj);
    ~NodeEditorWindow();
    virtual void Draw() override;

    void Initialize();
    void Clear();
    void Load(const nlohmann::json &model);
    void Save(nlohmann::json &model);
    std::string Build();
    std::list<std::shared_ptr<Connection> > GetNodeConnections(const std::string &nodeId);
    std::string GetNodeEntryLabel(const std::string &nodeId);

    std::shared_ptr<BaseNodeWidget> GetSelectedNode();

private:
    IStoryManager &m_story;

    ed::EditorContext* m_context = nullptr;

    bool m_loaded{false};

    // key: Id
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
        static std::shared_ptr<BaseNodeWidget> create_func(const std::string &type, IStoryManager &proj) {
            return std::make_shared<NodeType>(type, proj);
        }
    };

    typedef std::shared_ptr<BaseNodeWidget> (*GenericCreator)(const std::string &type, IStoryManager &proj);
    typedef std::map<std::string, GenericCreator> Registry;
    Registry m_registry;

    template<class Derived>
    void registerNode(const std::string& key) {
        m_registry.insert(typename Registry::value_type(key, Factory<Derived>::create_func));
    }

    std::shared_ptr<BaseNodeWidget> createNode(const std::string& key, IStoryManager &proj) {
        typename Registry::const_iterator i = m_registry.find(key);
        if (i == m_registry.end()) {
            throw std::invalid_argument(std::string(__PRETTY_FUNCTION__) +
                                        ": key not registered");
        }
        else
        {
            return i->second(key, proj);
        }
    }


    ed::PinId GetInputPin(const std::string &modelNodeId, int pinIndex);
    ed::PinId GetOutputPin(const std::string &modelNodeId, int pinIndex);
    std::string FindFirstNode() const;
    std::string  GenerateNodeId();
    void CreateLink(const Connection &model, ed::PinId inId, ed::PinId outId);
    Connection LinkToModel(ed::PinId InputId, ed::PinId OutputId);
};

