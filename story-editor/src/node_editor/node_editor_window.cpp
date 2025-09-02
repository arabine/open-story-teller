#include "node_editor_window.h"


#include "imgui.h"
#include "imgui_internal.h"
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <sstream>
#include "IconsFontAwesome5_c.h"


#include "gui.h"
#include "uuid.h"

#include <stdexcept> // for std::runtime_error
#define JSON_ASSERT(x) \
if (!(x)) { \
        throw std::runtime_error("Assertion failed: " #x); \
}
#include "json.hpp"


NodeEditorWindow::NodeEditorWindow(IStoryManager &manager, NodesFactory &factory, NodeWidgetFactory &widgetFactory, IStoryProject::Type type)
    : WindowBase(type == IStoryProject::Type::PROJECT_TYPE_STORY ? "Story editor" : "Module editor")
    , m_manager(manager)
    , m_nodesFactory(factory)
    , m_widgetFactory(widgetFactory)
    , m_editorType(type)
{


}
 
NodeEditorWindow::~NodeEditorWindow()
{
    Clear();
}

void NodeEditorWindow::Clear()
{
    m_pages.clear();
    m_story.reset();
    m_callStack.clear();
}

void NodeEditorWindow::Initialize()
{
    Clear();
}

void NodeEditorWindow::InitializeProject()
{
    // Always ensure a main page exists (this matches StoryProject::New)
    if (!m_story->GetPage(m_story->MainUuid()))
    {
        m_story->CreatePage(m_story->MainUuid());
    }
    m_currentPage = std::make_shared<NodeEditorPage>(m_story->MainUuid(), "Main");
    m_pages.push_back(m_currentPage);
    m_callStack.push_back(m_currentPage);
  
    m_currentPage->Select();
}

void NodeEditorWindow::LoadPage(const std::string &uuid, const std::string &name)
{
    // On cherche la page correspondante dans la std::list
    // Si elle n'existe pas, c'est une nouvelle fonction
    auto it = std::find_if(m_pages.begin(), m_pages.end(), [uuid](std::shared_ptr<NodeEditorPage> p) { return p->Uuid() == uuid; });

    std::shared_ptr<NodeEditorPage> page;

    if (it == m_pages.end())
    {
        // New page
        page = std::make_shared<NodeEditorPage>(uuid, name);
    }
    else
    {
        page = *it;
    }
    
    if (m_currentPage->Uuid() != uuid)
    {
        m_currentPage = page;
        m_currentPage->Select();
        m_callStack.push_back(m_currentPage); // show current page in call stack
    }
}

// ed::PinId NodeEditorWindow::GetInputPin(const std::string &modelNodeId, int pinIndex)
// {
//     return m_currentPage->GetInputPin(modelNodeId, pinIndex);
// }

// ed::PinId NodeEditorWindow::GetOutputPin(const std::string &modelNodeId, int pinIndex)
// {
//     return m_currentPage->GetOutputPin(modelNodeId, pinIndex);
// }

void NodeEditorWindow::Load(std::shared_ptr<StoryProject> story)
{
    m_story = story;

    if (m_story)
    {
        try {

            BaseNodeWidget::InitId();
            InitializeProject();

            auto [node_begin, node_end] = m_story->Nodes(m_currentPage->Uuid());

            int i = 0;

            for (auto it = node_begin; it != node_end; ++it)
            {
                auto n = m_widgetFactory.CreateNodeWidget((*it)->GetType(), m_manager, (*it));
                if (n)
                {
                    n->Initialize();
                //    n->SetOutputs(m_story->OutputsCount((*it)->GetId())); // il faut que les noeuds aient une bonne taille de outputs avant de créer les liens
                    // m_currentPage->AddNode(n);
                }
                else
                {
                    throw std::logic_error(std::string("No registered model with name ") + (*it)->GetType());
                }

                std::cout << "Created " << ++i << " node" << std::endl;
            }
            auto [link_begin, link_end] = m_story->Links(m_currentPage->Uuid());

            for (auto it = link_begin; it != link_end; ++it)
            {
                    // CreateLink(*it,
                    //         GetInputPin((*it)->inNodeId, (*it)->inPortIndex),
                    //         GetOutputPin((*it)->outNodeId, (*it)->outPortIndex));
            }

            m_loaded = true;
        }
        catch(std::exception &e)
        {
            std::cout << "(NodeEditorWindow::Load) " << e.what() << std::endl;
        }
    }

    // std::cout << "Loaded " << m_currentPage->m_nodes.size() << " nodes, " << m_currentPage->m_links.size() << " links" << std::endl;
  
}

void NodeEditorWindow::SaveNodePositions()
{
    
}

void NodeEditorWindow::SaveNodesToProject()
{
    // Pour toutes les pages
    for (const auto& page : m_pages)
    {
        // Clear current project nodes and links
        m_story->Clear();

        auto currentPage = m_story->CreatePage(page->Uuid());

        // On récupère les noeuds de la page
        for (const auto& node : page->GetNodes())
        {
            // On les ajoute au projet
            // currentPage->AddNode(node);
        }

        // On récupère tous les liens de la page
        for (const auto& link : page->GetLinks())
        {
            // On les ajoute au projet
            // currentPage->AddLink(link);
        }
    }
}

void NodeEditorWindow::OpenFunction(const std::string &uuid, const std::string &name)
{
    m_newPageUuid = uuid;
    m_newPageName = name;
}

// void NodeEditorWindow::CreateLink(std::shared_ptr<Connection> model, ed::PinId inId, ed::PinId outId)
// {

    // auto conn = std::make_shared<LinkInfo>();

    // conn->model = model;

    // // ImGui stuff for links
    // conn->ed_link->Id = BaseNodeWidget::GetNextId();
    // conn->ed_link->InputId = inId;
    // conn->ed_link->OutputId = outId;

    // Since we accepted new link, lets add one to our list of links.
    // m_currentPage->m_links.push_back(conn);
// }

/*
bool NodeEditorWindow::FillConnection(std::shared_ptr<Connection> c, ed::PinId pinId)
{
    bool success = false;
    // std::string nodeId;
    // int nodeIndex;
    // int ret = m_currentPage->FindNodeAndPin(pinId, nodeIndex, nodeId);
    // if (ret > 0)
    // {
    //     if (ret == 1)
    //     {
    //         c->outNodeId = nodeId;
    //         c->outPortIndex = nodeIndex;
    //     }
    //     else
    //     {
    //         c->inNodeId = nodeId;
    //         c->inPortIndex = nodeIndex;
    //     }
    //     success = true;
    // }
    return success;
}
*/

std::shared_ptr<BaseNodeWidget> NodeEditorWindow::GetSelectedNode()
{
    if (!m_currentPage)
    {
        return nullptr; // No current page, nothing to select
    }

    std::shared_ptr<BaseNodeWidget> selected;

    m_currentPage->Select();
    selected = m_currentPage->GetSelectedNode();

    return selected;
}


void NodeEditorWindow::Draw()
{
    // Check if we need to load a new page
    // Typically if we are in a function and we want to open a new one
    if (!m_newPageUuid.empty())
    {
        LoadPage(m_newPageUuid, m_newPageName);
        m_newPageUuid.clear();
    }

    if (WindowBase::BeginDraw())
    {
        if (m_currentPage)
        {
            m_currentPage->Select();

            ToolbarUI();

            m_currentPage->Draw(m_nodesFactory, m_widgetFactory, m_manager);

           
/*
            ed::Begin(m_currentPage->Uuid().data(), ImVec2(0.0, 0.0f));

            // Draw our nodes
            m_currentPage->Draw();

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
                                    m_story->AddConnection(m_currentPage->Uuid(), c);

                                    CreateLink(c, startId, endId);

                                    // Draw new link.
                                    ed::Link(m_currentPage->m_links.back()->ed_link->Id, startId, endId);
                                }
                            }
                    }

                    // You may choose to reject connection between these nodes
                    // by calling ed::RejectNewItem(). This will allow editor to give
                    // visual feedback by changing link thickness and color.
                }
                }

                ed::EndCreate(); // Wraps up object creation action handling.
            }
            


            // Handle deletion action
            if (ed::BeginDelete())
            {
                ed::NodeId nodeId = 0;
                while (ed::QueryDeletedNode(&nodeId))
                {
                    if (ed::AcceptDeletedItem())
                    {
                        std::shared_ptr<BaseNodeWidget> node;
                        if (m_currentPage->GetNode(nodeId, node))
                        {
                            // First delete model, then current entry
                            m_story->DeleteNode(m_currentPage->Uuid(), node->Base()->GetId());
                            m_currentPage->DeleteNode(nodeId);
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
                        std::shared_ptr<Connection> model;
                        if (m_currentPage->GetModel(deletedLinkId, model))
                        {
                            m_story->DeleteLink(m_currentPage->Uuid(), model);
                            m_currentPage->EraseLink(deletedLinkId);
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
                std::shared_ptr<BaseNode> base;
                auto nodeTypes = m_nodesFactory.ListOfNodes();

                for (auto &type : nodeTypes)
                {
                    if (ImGui::MenuItem(type.name.c_str()))
                    {
                        base = m_nodesFactory.CreateNode(type.uuid);
                        if (base)
                        {
                            m_story->AddNode(m_currentPage->Uuid(), base);
                            auto n = CreateNodeWidget(type.uuid, m_manager, base);
                            if (n)
                            {
                                n->Base()->SetPosition(newNodePostion.x, newNodePostion.y);
                                n->Initialize();
                                m_currentPage->AddNode(n);
                            }
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
            */
        }
        else
        {
            // Set background color to light gray
            // ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); 

            ImGui::Text("Please load or create a project.");

            ImGui::PopStyleColor(1); // Pop both colors
        }

    }

    WindowBase::EndDraw();
}

void NodeEditorWindow::ToolbarUI()
{
    // auto& io = ImGui::GetIO();
    // ImVec2 window_pos = ImGui::GetWindowPos();
    // ImVec2 window_size = ImGui::GetWindowSize();
    // ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove;

    // ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background

    // ImGui::Begin("TOOLBAR", NULL, window_flags);


    ImGui::SetCursorPos(ImVec2(10, 40));
    ImGui::BeginChild("ToolbarChild", ImVec2(0, ImGui::GetFrameHeightWithSpacing() * 1.5f), false, ImGuiWindowFlags_NoScrollbar);

    ImGui::PushID(m_editorType);

    // Draw call stack, each function is a button
    for (auto page : m_callStack)
    {
        if (ImGui::Button(page->Name().data()))
        {
            if (page->Uuid() != m_currentPage->Uuid())
            {
             
                // Erase all pages after this iterator
                auto it = std::find(m_callStack.begin(), m_callStack.end(), page);
                m_callStack.erase(it, m_callStack.end());

                LoadPage(page->Uuid().data(), page->Name().data());
                break;
            }
            
        }
        ImGui::SameLine();
        ImGui::Text(">");
        ImGui::SameLine();
    }

    ImGui::PopID();

    // ImGui::End();
    
    ImGui::EndChild(); // Fin de la ChildWindow de la barre d'outils

    ImGui::SetCursorPos(ImVec2(0, 0));


    // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    // {
    //     io.ConfigViewportsNoDecoration = false;
    // }
}
