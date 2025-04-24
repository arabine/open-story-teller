#include "print_node.h"
#include "story_project.h"
#include "connection.h"
#include "sys_lib.h"
#include "variable.h"

PrintNode::PrintNode(const std::string &type)
    : BaseNode(type, "Print Node")
{
    // Create empty variable in memory
    auto v = std::make_shared<Variable>(m_label);
    v->SetTextValue("");
    v->SetConstant(true);
    m_label = v->GetLabel();
    m_variables[m_label] = v;
}


void PrintNode::Initialize()
{

}
