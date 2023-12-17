#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <set>


#include <imgui_node_editor.h>
#include "base_node.h"
#include "window_base.h"
#include "i_story_project.h"
#include "story_project.h"
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
    struct LinkInfo
    {
        // Stuff from ImGuiNodeEditor
        ed::LinkId Id;
        ed::PinId  InputId;
        ed::PinId  OutputId;

        // Stuff from the project.json file, our model
        Connection model;
    };

    NodeEditorWindow(IStoryProject &proj);
    ~NodeEditorWindow();
    virtual void Draw() override;

    void Initialize();
    void Clear();
    void Load(const nlohmann::json &model);
    void Save(nlohmann::json &model);

    std::shared_ptr<BaseNode> GetSelectedNode();
    std::string GenerateConstants();
private:
    IStoryProject &m_project;

    ed::EditorContext* m_context = nullptr;

    // key: Id
    std::map<unsigned long, std::shared_ptr<BaseNode>>    m_nodes;
    std::vector<std::shared_ptr<LinkInfo>>   m_links;                // List of live links. It is dynamic unless you want to create read-only view over nodes.
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
        static std::shared_ptr<BaseNode> create_func(const std::string &title, IStoryProject &proj) {
            return std::make_shared<NodeType>(title, proj);
        }
    };

    typedef std::shared_ptr<BaseNode> (*GenericCreator)(const std::string &title, IStoryProject &proj);
    typedef std::map<std::string, GenericCreator> Registry;
    Registry m_registry;

    template<class Derived>
    void registerNode(const std::string& key) {
        m_registry.insert(typename Registry::value_type(key, Factory<Derived>::create_func));
    }

    std::shared_ptr<BaseNode> createNode(const std::string& key, const std::string &title, IStoryProject &proj) {
        typename Registry::const_iterator i = m_registry.find(key);
        if (i == m_registry.end()) {
            throw std::invalid_argument(std::string(__PRETTY_FUNCTION__) +
                                        ": key not registered");
        }
        else return i->second(title, proj);
    }

    void LoadNode(const nlohmann::json &nodeJson);
    ed::PinId GetInputPin(unsigned long modelNodeId, int pinIndex);
    ed::PinId GetOutputPin(unsigned long modelNodeId, int pinIndex);
    std::string ChoiceLabel() const;
    std::string EntryLabel() const;
};

