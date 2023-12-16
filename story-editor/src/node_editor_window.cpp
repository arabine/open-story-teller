#include "node_editor_window.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <iostream>
#include <cstdint>
#include <algorithm>
#include "IconsFontAwesome5_c.h"

#include "media_node.h"
#include "gui.h"


NodeEditorWindow::NodeEditorWindow(IStoryProject &proj)
    : WindowBase("Node editor")
    , m_project(proj)
{

    registerNode<MediaNode>("media-node");
}

NodeEditorWindow::~NodeEditorWindow()
{
    ed::DestroyEditor(m_context);
}

void NodeEditorWindow::Initialize()
{
    ed::Config config;
    config.SettingsFile = "Widgets.json";
    m_context = ed::CreateEditor(&config);

    ed::SetCurrentEditor(m_context);
}

void NodeEditorWindow::Clear()
{
    m_nodes.clear();
}



void NodeEditorWindow::LoadNode(const nlohmann::json &nodeJson)
{
    try
    {
        int restoredNodeId = nodeJson["id"].get<int>();
        nlohmann::json internalDataJson = nodeJson["internal-data"];
        std::string type = nodeJson["type"].get<std::string>();

        auto n = createNode(type, "", m_project);
        if (n)
        {
            n->SetType(type); // FIXME: set type in createNode factory?
            n->SetId(restoredNodeId);
            nlohmann::json posJson = nodeJson["position"];
            n->SetOutputs(nodeJson["outPortCount"].get<int>());
            n->SetPosition(posJson["x"].get<int>(), posJson["y"].get<int>());
            n->FromJson(internalDataJson);

            m_nodes[n->GetInternalId()] = n;
        }
        else
        {
            throw std::logic_error(std::string("No registered model with name ") + type);
        }
    }
    catch (std::exception&  e)
    {
        std::cout << "ERROR: " << e.what() << std::endl;
    }

}


ed::PinId NodeEditorWindow::GetInputPin(unsigned long modelNodeId, int pinIndex)
{
    ed::PinId id = -1;

    for (auto & n : m_nodes)
    {
        if (n.first == modelNodeId)
        {
            id = n.second->GetInputPinAt(pinIndex);
        }
    }

    return id;
}

ed::PinId NodeEditorWindow::GetOutputPin(unsigned long modelNodeId, int pinIndex)
{
    ed::PinId id = -1;

    for (auto & n : m_nodes)
    {
        if (n.first == modelNodeId)
        {
            id = n.second->GetOutputPinAt(pinIndex);
        }
    }

    return id;
}

void NodeEditorWindow::Load(const nlohmann::json &model)
{
    nlohmann::json nodesJsonArray = model["nodes"];

    BaseNode::InitId();
    m_nodes.clear();
    m_links.clear();

    for (auto& element : nodesJsonArray) {
        LoadNode(element);
    }

    std::cout << model.dump(4) << std::endl;

    nlohmann::json connectionJsonArray = model["connections"];

    for (auto& connection : connectionJsonArray)
    {
        auto conn = std::make_shared<LinkInfo>();

        // our model
        conn->model = connection.get<Connection>();


        // ImGui stuff for links
        conn->Id = BaseNode::GetNextId();
        conn->InputId = GetInputPin(conn->model.inNodeId, conn->model.inPortIndex);
        conn->OutputId = GetOutputPin(conn->model.outNodeId, conn->model.outPortIndex);

        // Since we accepted new link, lets add one to our list of links.
        m_links.push_back(conn);
    }
}

std::shared_ptr<BaseNode> NodeEditorWindow::GetSelectedNode()
{
    std::shared_ptr<BaseNode> selected;

    ed::SetCurrentEditor(m_context);
    if (ed::GetSelectedObjectCount() > 0)
    {
        ed::NodeId nId;
        int nodeCount = ed::GetSelectedNodes(&nId, 1);

        if (nodeCount > 0)
        {
            if (m_nodes.contains(nId.Get()))
            {
                selected = m_nodes[nId.Get()];
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
            ImGui::PushID(n.first);
            n.second->Draw();
            ImGui::PopID();
        }

        for (const auto& linkInfo : m_links)
        {
            ed::Link(linkInfo->Id, linkInfo->OutputId, linkInfo->InputId);
        }

        // Handle creation action, returns true if editor want to create new object (node or link)
        if (ed::BeginCreate())
        {
            ed::PinId inputPinId, outputPinId;
            if (ed::QueryNewLink(&inputPinId, &outputPinId))
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

               if (inputPinId && outputPinId) // both are valid, let's accept link
               {
                   // ed::AcceptNewItem() return true when user release mouse button.
                   if (ed::AcceptNewItem())
                   {
                       // Since we accepted new link, lets add one to our list of links.
//                       m_Links.push_back({ ed::LinkId(BaseNode::GetNextId()), inputPinId, outputPinId });

                       // Draw new link.
//                       ed::Link(m_Links.back().Id, m_Links.back().InputId, m_Links.back().OutputId);
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
            // There may be many links marked for deletion, let's loop over them.
            ed::LinkId deletedLinkId;
            while (ed::QueryDeletedLink(&deletedLinkId))
            {
               // If you agree that link can be deleted, accept deletion.
               if (ed::AcceptDeletedItem())
               {

                   m_links.erase(std::remove_if(m_links.begin(),
                                  m_links.end(),
                                                [deletedLinkId](std::shared_ptr<LinkInfo> inf) { return inf->Id == deletedLinkId; }));
               }

               // You may reject link deletion by calling:
               // ed::RejectDeletedItem();
            }
        }
        ed::EndDelete(); // Wrap up deletion action


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
