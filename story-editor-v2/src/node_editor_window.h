#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <set>


#include <imgui_node_editor.h>
#include "base_node.h"
#include "window_base.h"

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
        ed::LinkId Id;
        ed::PinId  InputId;
        ed::PinId  OutputId;
    };

    NodeEditorWindow();
    ~NodeEditorWindow();
    void Draw(const char *title, bool *p_open);

    void Initialize();
    void Clear();
private:
    ed::EditorContext* m_context = nullptr;

    std::vector<std::shared_ptr<BaseNode>>    m_nodes;



    ImVector<LinkInfo>   m_Links;                // List of live links. It is dynamic unless you want to create read-only view over nodes.
    int                  m_NextLinkId = 100;     // Counter to help generate link ids. In real application this will probably based on pointer to user data structure.
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
/*
    void BuildNodes()
    {
        for (auto& node : m_Nodes)
            BuildNode(&node);
    }

    Node* SpawnBranchNode()
    {
        m_Nodes.emplace_back(GetNextId(), "Branch");
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "Condition", PinType::Bool);
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "True", PinType::Flow);
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "False", PinType::Flow);

        BuildNode(&m_Nodes.back());

        return &m_Nodes.back();
    }
*/

};

