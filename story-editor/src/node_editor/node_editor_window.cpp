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
        auto storyPage = m_story->GetPage(uuid);
        if (!storyPage) {
            storyPage = m_story->CreatePage(uuid);
        }
        storyPage->SetName(name);
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
    
    if (!m_story)
    {
        std::cout << "Cannot load null story" << std::endl;
        return;
    }

    try
    {
        // Clear existing pages
        m_pages.clear();
        m_currentPage.reset();
        
        // Load all pages from the project
        auto [nodesBegin, nodesEnd] = m_story->Nodes(m_story->MainUuid());
        auto [linksBegin, linksEnd] = m_story->Links(m_story->MainUuid());
        
        // Create the main page
        auto page = std::make_shared<NodeEditorPage>(m_story->MainUuid(), "Main");
        m_pages.push_back(page);
        m_currentPage = page;
        
        // Map to store node UUID -> ImNodeFlow UID mapping
        std::map<std::string, ImFlow::NodeUID> nodeUidMap;
        
        // 1. Load all nodes from the project into ImNodeFlow
        for (auto it = nodesBegin; it != nodesEnd; ++it)
        {
            auto baseNode = *it;
            if (!baseNode)
                continue;
                
            // Create the widget for this node
            auto widget = m_widgetFactory.CreateNodeWidget(
                baseNode->GetType(), 
                m_manager, 
                baseNode
            );
            
            if (!widget)
            {
                std::cout << "Failed to create widget for node type: " 
                          << baseNode->GetType() << std::endl;
                continue;
            }
            
            // Initialize the widget
            widget->Initialize();
            
            // Create a NodeDelegate in ImNodeFlow
            ImVec2 nodePos(baseNode->GetX(), baseNode->GetY());
            auto delegate = page->mINF.addNode<NodeDelegate>(nodePos);
            
            // Link the delegate with the widget
            delegate->SetWidget(widget);
            
            // Store the mapping between project node UUID and ImNodeFlow UID
            nodeUidMap[baseNode->GetId()] = delegate->getUID();
            
            std::cout << "Loaded node: " << baseNode->GetType() 
                      << " at (" << nodePos.x << ", " << nodePos.y << ")" << std::endl;
        }
        
        // 2. Load all connections/links
        for (auto it = linksBegin; it != linksEnd; ++it)
        {
            auto connection = *it;
            if (!connection)
                continue;
                
            // Find the source and target nodes in ImNodeFlow
            auto sourceUidIt = nodeUidMap.find(connection->outNodeId);
            auto targetUidIt = nodeUidMap.find(connection->inNodeId);
            
            if (sourceUidIt == nodeUidMap.end() || targetUidIt == nodeUidMap.end())
            {
                std::cout << "Warning: Cannot create link - node not found" << std::endl;
                continue;
            }
            
            // Get the ImNodeFlow nodes
            auto& nodes = page->mINF.getNodes();
            auto sourceNodeIt = nodes.find(sourceUidIt->second);
            auto targetNodeIt = nodes.find(targetUidIt->second);
            
            if (sourceNodeIt == nodes.end() || targetNodeIt == nodes.end())
            {
                std::cout << "Warning: Node UID not found in ImNodeFlow" << std::endl;
                continue;
            }
            
            auto sourceNode = sourceNodeIt->second;
            auto targetNode = targetNodeIt->second;
            
            // Get the pins from the nodes
            // Output pin from source node
            auto& sourcePins = sourceNode->getOuts();
            if (connection->outPortIndex >= static_cast<int>(sourcePins.size()))
            {
                std::cout << "Warning: Invalid output port index: " 
                          << connection->outPortIndex << std::endl;
                continue;
            }
            auto* sourcePin = sourcePins[connection->outPortIndex].get();
            
            // Input pin from target node
            auto& targetPins = targetNode->getIns();
            if (connection->inPortIndex >= static_cast<int>(targetPins.size()))
            {
                std::cout << "Warning: Invalid input port index: " 
                          << connection->inPortIndex << std::endl;
                continue;
            }
            auto* targetPin = targetPins[connection->inPortIndex].get();
            
            // Create the link in ImNodeFlow
            if (sourcePin && targetPin)
            {
                targetPin->createLink(sourcePin);
                std::cout << "Created link: " << connection->outNodeId 
                          << "[" << connection->outPortIndex << "] -> " 
                          << connection->inNodeId 
                          << "[" << connection->inPortIndex << "]" << std::endl;
            }
        }
        
        m_loaded = true;
        std::cout << "Loaded " << nodeUidMap.size() << " nodes successfully" << std::endl;
    }
    catch(std::exception &e)
    {
        std::cout << "(NodeEditorWindow::Load) Exception: " << e.what() << std::endl;
        m_loaded = false;
    }
}

void NodeEditorWindow::SaveNodePositions()
{
    
}

void NodeEditorWindow::SaveNodesToProject()
{
    if (!m_story)
    {
        std::cout << "Cannot save: no story project loaded" << std::endl;
        return;
    }

    // IMPORTANT: Ne PAS appeler Clear() car cela efface aussi les variables!
    // Au lieu de cela, on efface seulement les pages
    for (const auto& page : m_pages)
    {
        // Récupérer la page correspondante dans le projet
        auto projectPage = m_story->GetPage(page->Uuid());
        
        if (!projectPage) {
            // La page n'existe pas, la créer
            projectPage = m_story->CreatePage(page->Uuid());
        } else {
            // La page existe, vider son contenu (nodes et links)
            projectPage->Clear();
        }

        // 1. Save all nodes with their updated positions
        for (auto &nodeEntry : page->mINF.getNodes())
        {
            auto delegate = dynamic_cast<NodeDelegate*>(nodeEntry.second.get());
            if (!delegate)
                continue;

            auto widget = delegate->GetWidget();
            if (!widget)
                continue;

            auto baseNode = widget->Base();
            if (!baseNode)
                continue;

            // Update node position from ImNodeFlow
            ImVec2 nodePos = nodeEntry.second->getPos();
            baseNode->SetPosition(nodePos.x, nodePos.y);

            // Add node to project
            m_story->AddNode(projectPage->Uuid(), baseNode);

            std::cout << "Saved node: " << baseNode->GetId() 
                      << " at (" << nodePos.x << ", " << nodePos.y << ")" << std::endl;
        }

        // 2. Save all links/connections
        const auto& links = page->mINF.getLinks();
        
        for (const auto& weakLink : links)
        {
            auto link = weakLink.lock();
            if (!link)
                continue;

            // Get left (source) and right (target) pins
            auto* leftPin = link->left();
            auto* rightPin = link->right();

            if (!leftPin || !rightPin)
                continue;

            // Get the nodes that own these pins
            auto* leftNode = leftPin->getParent();
            auto* rightNode = rightPin->getParent();

            if (!leftNode || !rightNode)
                continue;

            // Cast to NodeDelegate to get the widget
            auto* leftDelegate = dynamic_cast<NodeDelegate*>(leftNode);
            auto* rightDelegate = dynamic_cast<NodeDelegate*>(rightNode);

            if (!leftDelegate || !rightDelegate)
                continue;

            auto leftWidget = leftDelegate->GetWidget();
            auto rightWidget = rightDelegate->GetWidget();

            if (!leftWidget || !rightWidget)
                continue;

            // Get the base nodes (model)
            auto leftBaseNode = leftWidget->Base();
            auto rightBaseNode = rightWidget->Base();

            if (!leftBaseNode || !rightBaseNode)
                continue;

            // Find the pin indices
            int leftPinIndex = -1;
            int rightPinIndex = -1;

            // Find output pin index on left node
            const auto& leftOuts = leftNode->getOuts();
            for (size_t i = 0; i < leftOuts.size(); ++i)
            {
                if (leftOuts[i].get() == leftPin)
                {
                    leftPinIndex = static_cast<int>(i);
                    break;
                }
            }

            // Find input pin index on right node
            const auto& rightIns = rightNode->getIns();
            for (size_t i = 0; i < rightIns.size(); ++i)
            {
                if (rightIns[i].get() == rightPin)
                {
                    rightPinIndex = static_cast<int>(i);
                    break;
                }
            }

            if (leftPinIndex < 0 || rightPinIndex < 0)
            {
                std::cout << "Warning: Could not find pin indices for connection" << std::endl;
                continue;
            }

            // Create the connection object
            auto connection = std::make_shared<Connection>();
            connection->outNodeId = leftBaseNode->GetId();
            connection->outPortIndex = leftPinIndex;
            connection->inNodeId = rightBaseNode->GetId();
            connection->inPortIndex = rightPinIndex;

            // Detect type 
            auto outputPort = leftBaseNode->GetOutputPort(leftPinIndex);
            if (outputPort.type == BaseNode::Port::Type::DATA_PORT) {
                connection->type = Connection::DATA_LINK;
            } else {
                connection->type = Connection::EXECUTION_LINK;
            }

//             // 🔍 DEBUG
// std::cout << "=== SAVE CONNECTION ===" << std::endl;
// std::cout << "  From: " << connection->outNodeId << "[" << connection->outPortIndex << "]" << std::endl;
// std::cout << "  To: " << connection->inNodeId << "[" << connection->inPortIndex << "]" << std::endl;
// std::cout << "  Output port type: " << (outputPort.type == BaseNode::Port::Type::DATA_PORT ? "DATA_PORT" : "EXECUTION_PORT") << std::endl;
// std::cout << "  Connection type: " << (connection->type == Connection::DATA_LINK ? "DATA_LINK" : "EXECUTION_LINK") << std::endl;
// std::cout << "========================" << std::endl;

            // Add connection to project
            m_story->AddConnection(projectPage->Uuid(), connection);

            std::cout << "Saved connection: " << connection->outNodeId 
                      << "[" << connection->outPortIndex << "] -> " 
                      << connection->inNodeId 
                      << "[" << connection->inPortIndex << "]" << std::endl;
        }
    }

    std::cout << "SaveNodesToProject completed successfully (variables preserved)" << std::endl;
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
    ImGui::EndChild(); // Fin de la ChildWindow de la barre d'outils
    ImGui::SetCursorPos(ImVec2(0, 0));
}
