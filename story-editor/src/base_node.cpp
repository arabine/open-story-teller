#include "base_node.h"
#include "uuid.h"

#include "IconsMaterialDesignIcons.h"

unsigned long BaseNode::s_nextId = 1;

BaseNode::BaseNode(const std::string &title, IStoryProject &proj)
    : m_project(proj)
{
  //  m_id = UUID().String();

    m_id = -1; // Story Project Node ID
    m_node = std::make_unique<Node>(GetNextId(), title.c_str()); // ImGui internal ID
}

void BaseNode::AddInput()
{
   m_node->Inputs.emplace_back(GetNextId(), "", PinType::Flow);
}

void BaseNode::AddOutputs(int num)
{
   for (int i = 0; i < num; i++)
   {
        m_node->Outputs.emplace_back(GetNextId(), "", PinType::Flow);
   }
}

void BaseNode::SetOutputs(uint32_t num)
{
   if (num > Outputs())
   {
        AddOutputs(num - Outputs());
   }
   else if (num < Outputs())
   {
        for (unsigned int i = 0; i < (Outputs() - num); i++)
        {
            DeleteOutput();
        }
   }
}

void BaseNode::DeleteOutput()
{
   m_node->Outputs.pop_back();
}

void BaseNode::SetPosition(int x, int y)
{
   m_pos.x = x;
   m_pos.y = y;
   m_firstFrame = true;
}

void BaseNode::FrameStart()
{
    ed::BeginNode(m_node->ID);

    if (m_firstFrame)
    {
        ed::SetNodePosition(m_node->ID, ImVec2(m_pos.x, m_pos.y));
    }
    m_firstFrame = false;
}

void BaseNode::FrameEnd()
{
    ed::EndNode();
}

void BaseNode::DrawPins()
{
    static const char *str = "#1 >";
    static float textWidth = ImGui::CalcTextSize(str).x;

    for (auto& input : m_node->Inputs)
    {
        ed::BeginPin(input.ID, ed::PinKind::Input);

        ImGui::Text( ICON_MDI_OCTAGON_OUTLINE " In" );
        ed::EndPin();
    }

    int i = 1;
    for (auto& output : m_node->Outputs)
    {
        ImGui::Dummy(ImVec2(320 - textWidth * 2, 0)); // Hacky magic number to space out the output pin.
        ImGui::SameLine();
        ed::BeginPin(output.ID, ed::PinKind::Output);
        ImGui::Text( "#%d " ICON_MDI_OCTAGON_OUTLINE, i++);
        ed::EndPin();
    }
}
