#include "properties_window.h"

PropertiesWindow::PropertiesWindow()
    : WindowBase("Properties")
{

}

void PropertiesWindow::Initialize() {

    int my_image_width = 0;
    int my_image_height = 0;

}

void PropertiesWindow::Draw() {

}

void PropertiesWindow::Draw(std::shared_ptr<IStoryProject> story)
{
    WindowBase::BeginDraw();
    ImGui::SetWindowSize(ImVec2(626, 744), ImGuiCond_FirstUseEver);


    ImGui::SeparatorText("Selected node");

    static  std::shared_ptr<BaseNodeWidget> prev;
    if (m_selectedNode)
    {
        static char buf1[100] = "";

        if (prev != m_selectedNode)
        {
            prev = m_selectedNode;
          //  auto t = m_selectedNode->Base()->GetTitle();
            strncpy (buf1, m_selectedNode->Base()->GetTitle().data(), sizeof(buf1)) ;
        }
        
        ImGui::InputText("Title",     buf1, 32);
        m_selectedNode->Base()->SetTitle(buf1);
        ImGui::Text("Node ID: %s", m_selectedNode->Base()->GetId().data());
        m_selectedNode->DrawProperties(story);
    }

    WindowBase::EndDraw();
}

void PropertiesWindow::SetSelectedNode(std::shared_ptr<BaseNodeWidget> node)
{
    m_selectedNode = node;
}
