#include "base_node_widget.h"
#include "uuid.h"

#include "IconsMaterialDesignIcons.h"

unsigned long BaseNodeWidget::s_nextId = 1;

BaseNodeWidget::BaseNodeWidget(const std::string &type, IStoryManager &proj)
    : BaseNode(type)
    , m_story(proj)
{
  //  m_id = UUID().String();
    m_node = std::make_unique<Node>(GetNextId(), ""); // ImGui internal ID
}

void BaseNodeWidget::AddInput()
{
   m_node->Inputs.emplace_back(GetNextId(), "", PinType::Flow);
}

void BaseNodeWidget::AddOutputs(int num)
{
   for (int i = 0; i < num; i++)
   {
        m_node->Outputs.emplace_back(GetNextId(), "", PinType::Flow);
   }
}

void BaseNodeWidget::SetOutputs(uint32_t num)
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

void BaseNodeWidget::DeleteOutput()
{
   m_node->Outputs.pop_back();
}


void BaseNodeWidget::Initialize()
{
    m_firstFrame = true;
    
}


void BaseNodeWidget::FrameStart()
{
    ed::BeginNode(m_node->ID);

    if (m_firstFrame)
    {
        // Use the parent node position, the one saved in the JSON project
        // FIXME: find a better way to do that?
        ed::SetNodePosition(m_node->ID, ImVec2(BaseNode::GetX(), BaseNode::GetY()));
    }
    m_firstFrame = false;
}

void BaseNodeWidget::FrameEnd()
{
    ed::EndNode();
}

void BaseNodeWidget::DrawPins()
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

float BaseNodeWidget::GetX() const
{
    auto pos = GetNodePosition(m_node->ID);
    return pos.x;
}

float BaseNodeWidget::GetY() const
{
    auto pos = GetNodePosition(m_node->ID);
    return pos.y;
}
