#include "node_editor_window.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "IconsFontAwesome5_c.h"

#include "media_node.h"
#include "Gui.h"


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

/*
        for (auto& node : m_Nodes)
        {
            ed::BeginNode(node.ID);
            ImGui::Text("Node A");
            for (auto& input : node.Inputs)
            {
               ed::BeginPin(input.ID, ed::PinKind::Input);
               ImGui::Text("-> In");
               ed::EndPin();
            }

            for (auto& output : node.Outputs)
            {
               ed::BeginPin(output.ID, ed::PinKind::Output);
               ImGui::Text("Out ->");
               ed::EndPin();
            }

            ed::EndNode();
        }
*/
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
