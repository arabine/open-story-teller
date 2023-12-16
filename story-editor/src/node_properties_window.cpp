#include "node_properties_window.h"
#include "gui.h"

NodePropertiesWindow::NodePropertiesWindow()
    : WindowBase("Properties")
{

}

void NodePropertiesWindow::Initialize() {

    int my_image_width = 0;
    int my_image_height = 0;

}

void NodePropertiesWindow::Draw()
{
//    if (!IsVisible())
//    {
//        return;
//    }


    WindowBase::BeginDraw();
    ImGui::SetWindowSize(ImVec2(626, 744), ImGuiCond_FirstUseEver);


    static char buf1[32] = ""; ImGui::InputText("Title",     buf1, 32);

    if (m_selectedNode)
    {
        m_selectedNode->DrawProperties();
    }

    WindowBase::EndDraw();
}

void NodePropertiesWindow::SetSelectedNode(std::shared_ptr<BaseNode> node)
{
    m_selectedNode = node;
}
