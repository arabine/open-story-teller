#pragma once

#include <map>
#include <set>
#include <list>
#include <utility>

#include "base_node_widget.h"
#include "nodes_factory.h"
#include "node_widget_factory.h"
#include "i_story_manager.h"
#include "ImNodeFlow.h"

class SimpleSum : public ImFlow::BaseNode {
public:
    SimpleSum() {
        setTitle("Simple sum");
        setStyle(ImFlow::NodeStyle::green());
        ImFlow::BaseNode::addIN<int>("In", 0, ImFlow::ConnectionFilter::SameType());
        ImFlow::BaseNode::addOUT<int>("Out", nullptr)->behaviour([this]() { return getInVal<int>("In") + m_valB; });
    }

    void draw() override {
        ImGui::SetNextItemWidth(100.f);
        ImGui::InputInt("##ValB", &m_valB);
    }

private:
    int m_valB = 0;
};


// Generic delegate
class NodeDelegate : public ImFlow::BaseNode {
public:
    NodeDelegate()
    {

    }

    void SetWidget(std::shared_ptr<BaseNodeWidget> widget) {
        m_widget = widget;

        // Initialize delegate
        setTitle(m_widget->GetTitle());
        setStyle(ImFlow::NodeStyle::green());

        // Add Sync input if it is an executable node
        if (m_widget->HasSync())
        {
            ImFlow::BaseNode::addIN<int>(">", 0, ImFlow::ConnectionFilter::SameType());
        }

        // Add inputs
        for (int i = 0; i < m_widget->Inputs(); ++i) {

            auto port = m_widget->Base()->GetInputPort(i);

            if (port.customSocketIcon)
            {
                ImFlow::BaseNode::addIN<int>("In" + std::to_string(i), 0, ImFlow::ConnectionFilter::SameType())->renderer([this, i](ImFlow::Pin* p) {
                   Nw::Pin pin;
                    pin.index = i;
                    pin.isConnected = p->isConnected();
                    pin.pinKind = Nw::PinKind::Input;
                    pin.pinPoint = p->pinPoint();
                    pin.pos = p->getPos();
                    pin.size =  p->getSize();

                    m_widget->DrawSocket(pin);
                });
            }
            else
            {
                ImFlow::BaseNode::addIN<int>("In" + std::to_string(i), 0, ImFlow::ConnectionFilter::SameType());
            }
        }

        // Add outputs
        for (int i = 0; i < m_widget->Outputs(); ++i)
        {
            auto port = m_widget->Base()->GetOutputPort(i);
            if (port.customSocketIcon)
            {
                ImFlow::BaseNode::addOUT<int>("Out" + std::to_string(i), nullptr)->renderer([this, i](ImFlow::Pin* p) {
                
                    Nw::Pin pin;
                    pin.index = i;
                    pin.isConnected = p->isConnected();
                    pin.pinKind = Nw::PinKind::Output;
                    pin.pinPoint = p->pinPoint();
                    pin.pos = p->getPos();
                    pin.size =  p->getSize();

                    m_widget->DrawSocket(pin);
                });
            }
            else
            {
                ImFlow::BaseNode::addOUT<int>("Out" + std::to_string(i), nullptr)->behaviour([this, i]() { return getInVal<int>("In" + std::to_string(i)) + m_valB; });
            }
        }
    }

    std::shared_ptr<BaseNodeWidget> GetWidget() {
        return m_widget;
    }

    void draw() override {
        if (m_widget)
        {
            m_widget->Draw();
        }
        
        // ImGui::SetNextItemWidth(100.f);
        // ImGui::InputInt("##ValB", &m_valB);
    }

private:
    int m_valB = 0;
    std::shared_ptr<BaseNodeWidget> m_widget;
};



struct NodeEditorPage : public  ImFlow::BaseNode 
{
    ImFlow::ImNodeFlow mINF;

    NodeEditorPage(const std::string &uuid, const std::string &name) 
        : m_uuid(uuid)
        , m_name(name)
    {
        
        mINF.setSize({500, 500});
    }

    ~NodeEditorPage() {
        // Clear();
    }

    std::string_view Name() const {
        return m_name;
    }

    std::string_view Uuid() const {
        return m_uuid;
    }

    std::list<std::shared_ptr<BaseNodeWidget>> GetNodes()
    {
        std::list<std::shared_ptr<BaseNodeWidget>> nlist;
        // std::unordered_map<ImFlow::NodeUID, std::shared_ptr<BaseNode>>&
        for (auto &node : mINF.getNodes())
        {
            auto delegate = dynamic_cast<NodeDelegate*>(node.second.get());

            if (delegate == nullptr)
                continue;
            nlist.push_back(delegate->GetWidget());
        }

        return nlist;
    }

    std::list<std::shared_ptr<Connection>> GetLinks() {

        std::list<std::shared_ptr<Connection>> links;

        // const std::vector<std::weak_ptr<Link>>& getLinks()
        for (auto &link : mINF.getLinks())
        {
            auto linkInfo = std::make_shared<Connection>();

            links.push_back(linkInfo);
        }

        return links;
    }

    void Select()
    {
    }

    void Draw(NodesFactory &nodesFactory, NodeWidgetFactory &widgetFactory, IStoryManager &storyManager)
    {
        mINF.update();

        auto openPopupPosition = ImGui::GetMousePos();
        mINF.rightClickPopUpContent([this, openPopupPosition, &nodesFactory, &widgetFactory, &storyManager](ImFlow::BaseNode* node){
            // std::cout << "Right-clicked on node: " << node->getName() << std::endl;

            auto newNodePosition = mINF.screen2grid(openPopupPosition);
            auto nodeTypes = nodesFactory.ListOfNodes();

            for (auto &type : nodeTypes)
            {
                if (ImGui::MenuItem(type.name.c_str()))
                {
                    auto base = nodesFactory.CreateNode(type.uuid);
                    if (base)
                    {
                        auto n = widgetFactory.CreateNodeWidget(type.uuid, storyManager, base);
                        if (n)
                        {
                            // Create delegate
                            auto delegate =  mINF.placeNode<NodeDelegate>();
                            // Link with the widget
                            delegate->SetWidget(n);

                            n->Base()->SetPosition(newNodePosition.x, newNodePosition.y);
                            n->Initialize();
                           // AddNode(n);
                        }
                    }
                }
            }
        });

        /*
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
            */
    }

    // bool GetNode(const ed::NodeId &nodeId, std::shared_ptr<BaseNodeWidget> &node) {
    //     for (const auto & n : m_nodes)
    //     {
    //         if (n->GetInternalId() == nodeId.Get())
    //         {
    //             node = n;
    //             return true;
    //         }
    //     }
    //     return false;
    // }

    // void DeleteNode(const ed::NodeId &nodeId) {
    //     m_nodes.remove_if([nodeId](const std::shared_ptr<BaseNodeWidget>& node) {
    //         return node->GetInternalId() == nodeId.Get();
    //     });
    // }

    std::shared_ptr<BaseNodeWidget> GetSelectedNode() {

        std::shared_ptr<BaseNodeWidget> selected;

        updatePublicStatus();

       // if (mINF.on_selected_node())
        {
            // Loop all nodes, get first selected
            for (auto &node : mINF.getNodes())
            {
                if (node.second->isSelected())
                {
                    // cast to delegate
                    auto delegate = dynamic_cast<NodeDelegate*>(node.second.get());
                    if (delegate)
                    {
                        //get widget
                        selected = delegate->GetWidget();
                    }
                    break;
                }
            }
        }

        

        // if (ed::GetSelectedObjectCount() > 0)
        // {
        //     ed::NodeId nId;
        //     int nodeCount = ed::GetSelectedNodes(&nId, 1);

        //     if (nodeCount > 0)
        //     {
        //         for (auto & n : m_nodes)
        //         {
        //             if (n->GetInternalId() == nId.Get())
        //             {
        //                 selected = n;
        //             }
        //         }
        //     }
        // }
        return selected;
    }

    // void AddNode(std::shared_ptr<BaseNodeWidget> node) {
    //     m_nodes.push_back(node);
    // }

    // void Clear() {
    //     m_nodes.clear();
    //     m_links.clear();
    // }
/*
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

    */

private:
    std::string m_uuid;
    std::string m_name;

};
