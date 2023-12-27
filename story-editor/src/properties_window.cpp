#include "properties_window.h"
#include "gui.h"

PropertiesWindow::PropertiesWindow()
    : WindowBase("Properties")
{

}

void PropertiesWindow::Initialize() {

    int my_image_width = 0;
    int my_image_height = 0;

}

void PropertiesWindow::Draw()
{
//    if (!IsVisible())
//    {
//        return;
//    }


    WindowBase::BeginDraw();
    ImGui::SetWindowSize(ImVec2(626, 744), ImGuiCond_FirstUseEver);

    ImGui::SeparatorText("Selected node");


    if (m_selectedNode)
    {
        static char buf1[32] = ""; ImGui::InputText("Title",     buf1, 32);
        ImGui::Text("Node ID: %lu", m_selectedNode->GetId());
        m_selectedNode->DrawProperties();
    }

    WindowBase::EndDraw();
}

void PropertiesWindow::SetSelectedNode(std::shared_ptr<BaseNode> node)
{
    m_selectedNode = node;
}
