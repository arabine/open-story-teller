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
#include "base_node.h"

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

        // Add inputs
        for (int i = 0; i < m_widget->Inputs(); ++i) {

            auto port = m_widget->Base()->GetInputPort(i);

            std::string label = (port.type == ::BaseNode::Port::Type::EXECUTION_PORT) ? "" : port.label;

            if (port.customSocketIcon)
            {
                ImFlow::BaseNode::addIN<int>(label, 0, ImFlow::ConnectionFilter::SameType())->renderer([this, i](ImFlow::Pin* p) {
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
                ImFlow::BaseNode::addIN<int>(label, 0, ImFlow::ConnectionFilter::SameType());
            }
        }

        // Add outputs
        for (int i = 0; i < m_widget->Outputs(); ++i)
        {
            auto port = m_widget->Base()->GetOutputPort(i);
            // Détermine le label : vide pour les ports d'exécution, utilise port.label pour les ports de données
            std::string label = (port.type == ::BaseNode::Port::Type::EXECUTION_PORT) ? "" : port.label;
            if (port.customSocketIcon)
            {
                ImFlow::BaseNode::addOUT<int>(label, nullptr)->renderer([this, i](ImFlow::Pin* p) {
                
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
                ImFlow::BaseNode::addOUT<int>(label, nullptr)->behaviour([this, i]() { return getInVal<int>("In" + std::to_string(i)) + m_valB; });
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
        
        mINF.setSize({0, 0});
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
    }

    std::shared_ptr<BaseNodeWidget> GetSelectedNode()
    {

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

        return selected;
    }

private:
    std::string m_uuid;
    std::string m_name;

};
