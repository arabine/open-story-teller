#include "node_editor_window.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "IconsFontAwesome5_c.h"

#include "media_node.h"
#include "gui.h"


NodeEditorWindow::NodeEditorWindow()
{

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

    auto n1 = std::make_shared<MediaNode>("Branch");
    n1->SetPosition(0, 0);
    m_nodes.push_back(n1);

    auto n2 = std::make_shared<MediaNode>("Branch 2");
    n2->SetPosition(100, 100);
    m_nodes.push_back(n2);

}

void NodeEditorWindow::Clear()
{
    m_nodes.clear();
}

void NodeEditorWindow::Draw(const char *title, bool *p_open)
{
    static bool resetDockspace = true;

    float menuHeight = 0;

    if(ImGui::BeginMainMenuBar())
    {
        menuHeight = ImGui::GetWindowSize().y;

        if (ImGui::BeginMenu("Actions"))
        {
            if(ImGui::MenuItem("Quit"))
            {
               // mEvent.ExitGame();
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    static ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;


    if (ImGui::Begin("EditorView", NULL, window_flags))
    {

        ed::SetCurrentEditor(m_context);
        ed::Begin("My Editor", ImVec2(0.0, 0.0f));

        for (auto & n : m_nodes)
        {
            n->Draw();
        }

        for (auto& linkInfo : m_Links)
        {
            ed::Link(linkInfo.Id, linkInfo.InputId, linkInfo.OutputId);
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
                       m_Links.push_back({ ed::LinkId(m_NextLinkId++), inputPinId, outputPinId });

                       // Draw new link.
                       ed::Link(m_Links.back().Id, m_Links.back().InputId, m_Links.back().OutputId);
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
                   // Then remove link from your data.
                   for (auto& link : m_Links)
                   {
                       if (link.Id == deletedLinkId)
                       {
                           m_Links.erase(&link);
                           break;
                       }
                   }
               }

               // You may reject link deletion by calling:
               // ed::RejectDeletedItem();
            }
        }
        ed::EndDelete(); // Wrap up deletion action


        ed::End();
        ed::SetCurrentEditor(nullptr);

    }

    ImGui::End();
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
