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
    m_label = v->GetLabel();
    m_variables[m_label] = v;
}


void PrintNode::Initialize()
{

}

std::string PrintNode::Build(IStoryPage &page, const StoryOptions &options, int nb_out_conns)
{
    return "";
}


