#pragma once

#include <map>
#include <set>
#include <list>
#include <utility>

#include <imgui_node_editor.h>
#include "base_node_widget.h"

namespace ed = ax::NodeEditor;

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


struct NodeEditorPage  {
    ed::Config config;
    ed::EditorContext* EditorContext;
    std::list<std::shared_ptr<BaseNodeWidget>>  m_nodes;
    std::list<std::shared_ptr<LinkInfo>> m_links;                // List of live links. It is dynamic unless you want to create read-only view over nodes.

    NodeEditorPage(const std::string &uuid, const std::string &name) 
        : m_uuid(uuid)
        , m_name(name)
    {
        config.SettingsFile = nullptr;
        config.SaveSettings = nullptr;
        config.LoadSettings = nullptr;
        EditorContext = ed::CreateEditor(&config);
    }

    std::string_view Name() const {
        return m_name;
    }

    std::string_view Uuid() const {
        return m_uuid;
    }

    ~NodeEditorPage() {
        ed::DestroyEditor(EditorContext);
        Clear();
    }

    void Select()
    {
        ed::SetCurrentEditor(EditorContext);
    }

    void Draw() {
        for (const auto & n : m_nodes)
        {
            ImGui::PushID(n->GetInternalId());
            n->Draw();
            ImGui::PopID();
        }

        for (const auto& linkInfo : m_links)
        {
            ed::Link(linkInfo->ed_link->Id, linkInfo->ed_link->OutputId, linkInfo->ed_link->InputId);
        }
    }

    bool GetNode(const ed::NodeId &nodeId, std::shared_ptr<BaseNodeWidget> &node) {
        for (const auto & n : m_nodes)
        {
            if (n->GetInternalId() == nodeId.Get())
            {
                node = n;
                return true;
            }
        }
        return false;
    }

    void DeleteNode(const ed::NodeId &nodeId) {
        m_nodes.remove_if([nodeId](const std::shared_ptr<BaseNodeWidget>& node) {
            return node->GetInternalId() == nodeId.Get();
        });
    }

    std::shared_ptr<BaseNodeWidget> GetSelectedNode() {

        std::shared_ptr<BaseNodeWidget> selected;

        if (ed::GetSelectedObjectCount() > 0)
        {
            ed::NodeId nId;
            int nodeCount = ed::GetSelectedNodes(&nId, 1);

            if (nodeCount > 0)
            {
                for (auto & n : m_nodes)
                {
                    if (n->GetInternalId() == nId.Get())
                    {
                        selected = n;
                    }
                }
            }
        }
        return selected;
    }

    void AddNode(std::shared_ptr<BaseNodeWidget> node) {
        m_nodes.push_back(node);
    }

    void Clear() {
        m_nodes.clear();
        m_links.clear();
    }

    bool GetModel(ed::LinkId linkId, std::shared_ptr<Connection> &model) {
        for (const auto& linkInfo : m_links)
        {
            if (linkInfo->ed_link->Id == linkId)
            {
                model = linkInfo->model;
                return true;
            }
        }
        return false;
    }

    void EraseLink(ed::LinkId linkId) {
        m_links.remove_if([linkId](const std::shared_ptr<LinkInfo>& linkInfo) {
            return linkInfo->ed_link->Id == linkId;
        });
    }

    // retourne 1 si c'est une sortie, 2 une entrée, 0 pas trouvé
    int FindNodeAndPin(ed::PinId pinId, int &foundIndex, std::string &foundNodeId)
    {
        int success = 0;
        for (const auto & n : m_nodes)
        {
            // std::cout << "---> Node: " << n->Base()->GetId() << std::endl;

            if (n->HasOnputPinId(pinId, foundIndex))
            {
                foundNodeId = n->Base()->GetId();
                success = 1;
                break;
            }

            if (n->HasInputPinId(pinId, foundIndex))
            {
                foundNodeId = n->Base()->GetId();
                success = 2;
                break;
            }
        }

        return success;
    }

    ed::PinId GetInputPin(const std::string &modelNodeId, int pinIndex)
    {
        ed::PinId id = 0;

        for (auto & n : m_nodes)
        {
            if (n->Base()->GetId() == modelNodeId)
            {
                id = n->GetInputPinAt(pinIndex);
                break;
            }
        }

        if (id.Get() == 0)
        {
            std::cout << "Invalid Id: " << modelNodeId << " input pin: " << pinIndex <<" not found" << std::endl;
        }

        return id;
    }

    ed::PinId GetOutputPin(const std::string &modelNodeId, int pinIndex)
{
        ed::PinId id = 0;

        for (auto & n : m_nodes)
        {
            if (n->Base()->GetId() == modelNodeId)
            {
                id = n->GetOutputPinAt(pinIndex);
                break;
            }
        }

        if (id.Get() == 0)
        {
            std::cout << "Invalid Id: " << modelNodeId << " output pin: " << pinIndex <<" not found" << std::endl;
        }

        return id;
    }

private:
    std::string m_uuid;
    std::string m_name;

};
