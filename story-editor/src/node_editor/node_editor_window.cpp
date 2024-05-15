#include "node_editor_window.h"


#include "imgui.h"
#include "imgui_internal.h"
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <sstream>
#include "IconsFontAwesome5_c.h"

#include "media_node_widget.h"
#include "gui.h"
#include "uuid.h"

#include <stdexcept> // for std::runtime_error
#define JSON_ASSERT(x) \
if (!(x)) { \
        throw std::runtime_error("Assertion failed: " #x); \
}
#include "json.hpp"


NodeEditorWindow::NodeEditorWindow(IStoryManager &manager)
    : WindowBase("Node editor")
    , m_manager(manager)
{

    registerNode<MediaNodeWidget>("media-node");
}

NodeEditorWindow::~NodeEditorWindow()
{
    ed::DestroyEditor(m_context);
}

void NodeEditorWindow::Initialize()
{
    ed::Config config;
    config.SettingsFile = nullptr;
    config.SaveSettings = nullptr;
    config.LoadSettings = nullptr;
    m_context = ed::CreateEditor(&config);

    ed::SetCurrentEditor(m_context);
}

void NodeEditorWindow::Clear()
{
    m_nodes.clear();
    m_links.clear();
    m_story.reset();
}

ed::PinId NodeEditorWindow::GetInputPin(const std::string &modelNodeId, int pinIndex)
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

ed::PinId NodeEditorWindow::GetOutputPin(const std::string &modelNodeId, int pinIndex)
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

void NodeEditorWindow::Load(std::shared_ptr<StoryProject> story)
{
    m_story = story;

    if (m_story)
    {
        try {

            BaseNodeWidget::InitId();
            m_nodes.clear();
            m_links.clear();

            auto [node_begin, node_end] = m_story->Nodes();

            int i = 0;

            for (auto it = node_begin; it != node_end; ++it)
            {
                auto n = CreateNodeWidget((*it)->GetType(), m_manager, (*it));
                if (n)
                {
                    n->Initialize();
                    n->SetOutputs(m_story->OutputsCount((*it)->GetId())); // il faut que les noeuds aient une bonne taille de outputs avant de créer les liens
                    m_nodes.push_back(n);
                }
                else
                {
                    throw std::logic_error(std::string("No registered model with name ") + (*it)->GetType());
                }

                std::cout << "Created " << ++i << " node" << std::endl;
            }
            auto [link_begin, link_end] = m_story->Links();

            for (auto it = link_begin; it != link_end; ++it)
            {
                    CreateLink(*it,
                            GetInputPin((*it)->inNodeId, (*it)->inPortIndex),
                            GetOutputPin((*it)->outNodeId, (*it)->outPortIndex));
            }

            m_loaded = true;
        }
        catch(std::exception &e)
        {
            std::cout << "(NodeEditorWindow::Load) " << e.what() << std::endl;
        }
    }

    std::cout << "Loaded " << m_nodes.size() << " nodes, " << m_links.size() << " links" << std::endl;
  
}

void NodeEditorWindow::SaveNodePositions()
{
    
}

void NodeEditorWindow::CreateLink(std::shared_ptr<Connection> model, ed::PinId inId, ed::PinId outId)
{
    auto conn = std::make_shared<LinkInfo>();

    conn->model = model;

    // ImGui stuff for links
    conn->ed_link->Id = BaseNodeWidget::GetNextId();
    conn->ed_link->InputId = inId;
    conn->ed_link->OutputId = outId;

    // Since we accepted new link, lets add one to our list of links.
    m_links.push_back(conn);
}

// retourne 1 si c'est une sortie, 2 une entrée, 0 pas trouvé
int NodeEditorWindow::FindNodeAndPin(ed::PinId pinId, int &foundIndex, std::string &foundNodeId)
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

bool NodeEditorWindow::FillConnection(std::shared_ptr<Connection> c, ed::PinId pinId)
{
    bool success = false;
    std::string nodeId;
    int nodeIndex;
    int ret = FindNodeAndPin(pinId, nodeIndex, nodeId);
    if (ret > 0)
    {
        if (ret == 1)
        {
            c->outNodeId = nodeId;
            c->outPortIndex = nodeIndex;
        }
        else
        {
            c->inNodeId = nodeId;
            c->inPortIndex = nodeIndex;
        }
        success = true;
    }
    return success;
}

/*
std::shared_ptr<Connection> NodeEditorWindow::LinkToModel(ed::PinId InputId, ed::PinId OutputId)
{
    auto c = std::make_shared<Connection>();
    int foundIndex = -1;
    for (const auto & n : m_nodes)
    {
        // std::cout << "---> Node: " << n->Base()->GetId() << std::endl;

        if (n->HasOnputPinId(OutputId, foundIndex))
        {
            c->outNodeId = n->Base()->GetId();
            c->outPortIndex = foundIndex;
        }

        if (n->HasInputPinId(InputId, foundIndex))
        {
            c->inNodeId = n->Base()->GetId();
            c->inPortIndex = foundIndex;
        }
    }

    return c;
}*/


std::shared_ptr<BaseNodeWidget> NodeEditorWindow::GetSelectedNode()
{
    std::shared_ptr<BaseNodeWidget> selected;

    ed::SetCurrentEditor(m_context);
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
    ed::SetCurrentEditor(nullptr);

    return selected;
}


void NodeEditorWindow::Draw()
{
    if (WindowBase::BeginDraw())
    {

        ed::SetCurrentEditor(m_context);
        ed::Begin("My Editor", ImVec2(0.0, 0.0f));


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

        // Handle creation action, returns true if editor want to create new object (node or link)
        if (ed::BeginCreate())
        {
            ed::PinId startId, endId;
            if (ed::QueryNewLink(&startId, &endId))
            {
               // QueryNewLink returns true if editor want to create new link between pins.
               //
               // Link can be created only for two valid pins, it is up to you to
               // validate if connection make sense. Editor is happy to make any.
               //
               // Link always goes from input to output. User may choose to drag
               // link from output pin or input pin. This determine which pin ids
               // are valid and which are not:
               //   * input valid, output invalid - user started to drag new ling from input pin
               //   * input invalid, output valid - user started to drag new ling from output pin
               //   * input valid, output valid   - user dragged link over other pin, can be validated

               if (startId && endId) // both are valid, let's accept link
               {
                   // ed::AcceptNewItem() return true when user release mouse button.
                   if (ed::AcceptNewItem())
                   {
                        auto c = std::make_shared<Connection>();

                        // On cherche à quel noeud appartien les pin (selon si le lien a été créé à partir d'une entrée ou d'une sortie)
                        if (FillConnection(c, startId))
                        {
                            if (FillConnection(c, endId))
                            {
                                m_story->AddConnection(c);

                                CreateLink(c, startId, endId);

                                // Draw new link.
                                ed::Link(m_links.back()->ed_link->Id, startId, endId);
                            }
                        }
                   }

                   // You may choose to reject connection between these nodes
                   // by calling ed::RejectNewItem(). This will allow editor to give
                   // visual feedback by changing link thickness and color.
               }
            }
        }
        ed::EndCreate(); // Wraps up object creation action handling.


        // Handle deletion action
        if (ed::BeginDelete())
        {
            ed::NodeId nodeId = 0;
            while (ed::QueryDeletedNode(&nodeId))
            {
                if (ed::AcceptDeletedItem())
                {
                    auto it = std::find_if(m_nodes.begin(), m_nodes.end(), [nodeId](std::shared_ptr<BaseNodeWidget> node) { return node->GetInternalId() == nodeId.Get(); });
                    if (it != m_nodes.end())
                    {
                        // First delete model, then current entry
                        m_manager.DeleteNode((*it)->Base()->GetId());
                        m_nodes.erase(it);
                    }
                }
            }


            // There may be many links marked for deletion, let's loop over them.
            ed::LinkId deletedLinkId;
            while (ed::QueryDeletedLink(&deletedLinkId))
            {
               // If you agree that link can be deleted, accept deletion.
               if (ed::AcceptDeletedItem())
               {

                    auto it = std::find_if(m_links.begin(), m_links.end(), [deletedLinkId](std::shared_ptr<LinkInfo> inf) { return inf->ed_link->Id == deletedLinkId; });
                    if (it != m_links.end())
                    {
                        // First delete model, then current entry
                        m_manager.DeleteLink((*it)->model);
                        m_links.erase(it);
                    }
               }

               // You may reject link deletion by calling:
               // ed::RejectDeletedItem();
            }
        }
        ed::EndDelete(); // Wrap up deletion action


        auto openPopupPosition = ImGui::GetMousePos();
        ed::Suspend();

        if (ed::ShowBackgroundContextMenu())
        {
            ImGui::OpenPopup("Create New Node");
        }

        if (ImGui::BeginPopup("Create New Node"))
        {
            auto newNodePostion = openPopupPosition;
            Node* node = nullptr;
            if (ImGui::MenuItem("Media Node"))
            {
                auto base = m_manager.CreateNode("media-node");
                if (base)
                {
                    auto n = CreateNodeWidget(base->GetType(), m_manager, base);
                    if (n)
                    {
                        n->Base()->SetPosition(newNodePostion.x, newNodePostion.y);
                        n->Initialize();
                        m_nodes.push_back(n);
                    }
                }
            }

            ImGui::EndPopup();
        }

        if (m_loaded)
        {
            ed::NavigateToContent();
            m_loaded = false;
        }

        ed::Resume();


        ed::End();
        ed::SetCurrentEditor(nullptr);

    }

    WindowBase::EndDraw();
}

void NodeEditorWindow::ToolbarUI()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(42, Gui::GetWindowSize().h));

    ImGuiWindowFlags window_flags = 0
                                    | ImGuiWindowFlags_NoTitleBar
                                    | ImGuiWindowFlags_NoResize
                                    | ImGuiWindowFlags_NoMove
                                    | ImGuiWindowFlags_NoScrollbar
                                    | ImGuiWindowFlags_NoSavedSettings
        ;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::Begin("TOOLBAR", NULL, window_flags);
    ImGui::PopStyleVar();

    if (ImGui::Button(ICON_FA_COG))
    {
        ImGui::OpenPopup("Options");
    }

    if (ImGui::Button(ICON_FA_SIGN_OUT_ALT))
    {
       // mEvent.ExitGame();
    }


    ImGui::End();
}
