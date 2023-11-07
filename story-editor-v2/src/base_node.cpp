#include "base_node.h"

#include "IconsMaterialDesignIcons.h"

int BaseNode::s_nextId = 1;

BaseNode::BaseNode(const std::string &title)
{
    m_id = UUID().String();

    m_node = std::make_unique<Node>(GetNextId(), title.c_str());

//    m_node->Inputs.emplace_back(GetNextId(), "", PinType::Flow);
//    m_node->Inputs.emplace_back(GetNextId(), "Condition", PinType::Bool);
//    m_node->Outputs.emplace_back(GetNextId(), "True", PinType::Flow);
//    m_node->Outputs.emplace_back(GetNextId(), "False", PinType::Flow);
}

void BaseNode::AddInput()
{
   m_node->Inputs.emplace_back(GetNextId(), "", PinType::Flow);
}

void BaseNode::AddOutput()
{
   m_node->Outputs.emplace_back(GetNextId(), "", PinType::Flow);
}

void BaseNode::DeleteOutput()
{
   m_node->Outputs.pop_back();
}

void BaseNode::SetPosition(int x, int y)
{
    ed::SetNodePosition(m_node->ID, ImVec2(0, 0));
}

void BaseNode::FrameStart()
{
    ed::BeginNode(m_node->ID);

//    ImGui::Text("Node A");
//    for (auto& input : m_node->Inputs)
//    {
//        ed::BeginPin(input.ID, ed::PinKind::Input);
//        ImGui::Text("-> In");
//        ed::EndPin();
//    }

//    for (auto& output : m_node->Outputs)
//    {
//        ed::BeginPin(output.ID, ed::PinKind::Output);
//        ImGui::Text("Out ->");
//        ed::EndPin();
//    }

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

        ImGui::Text( ICON_MDI_OCTAGON_OUTLINE " In"  );
        ed::EndPin();
    }

    for (auto& output : m_node->Outputs)
    {
        ImGui::Dummy(ImVec2(320 - textWidth * 2, 0)); // Hacky magic number to space out the output pin.
        ImGui::SameLine();
        ed::BeginPin(output.ID, ed::PinKind::Output);
        ImGui::Text( "#1 " ICON_MDI_OCTAGON_OUTLINE );
        ed::EndPin();
    }
}
