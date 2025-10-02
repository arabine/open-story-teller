#include "print_node.h"
#include "story_project.h"
#include "connection.h"
#include "sys_lib.h"
#include "variable.h"

PrintNode::PrintNode(const std::string &type)
    : BaseNode(type, "Print Node")
{
    // Create empty variable in memory for the format string
    auto v = std::make_shared<Variable>(m_label);
    v->SetConstant(true);
    m_label = v->GetLabel();
    m_variables[m_label] = v;

    SetBehavior(BaseNode::BEHAVIOR_EXECUTION);

    // Add execution input port (sync)
    AddInputPort(Port::EXECUTION_PORT, ">");
    
    // Add 4 data input ports for arguments
    for (int i = 0; i < MAX_INPUT_COUNT; ++i) {
        AddInputPort(Port::DATA_PORT, "arg" + std::to_string(i));
    }
    
    // Add execution output port
    AddOutputPort(Port::EXECUTION_PORT, ">");

    SetText("");
}

void PrintNode::Initialize()
{
    nlohmann::json j = GetInternalData();
    m_variables.at(m_label)->SetTextValue(j["text"].get<std::string>());
}

void PrintNode::SetText(const std::string &text)
{
    m_variables.at(m_label)->SetValue<std::string>(text);
    SetInternalData({{"text", text}});
}

std::string PrintNode::GetLabel() const
{
    return m_label;
}

std::string PrintNode::GetText() const
{
    return m_variables.at(m_label)->GetValue<std::string>();
}