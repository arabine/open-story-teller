#include "print_node.h"
#include "story_project.h"
#include "connection.h"
#include "sys_lib.h"


PrintNode::PrintNode(const std::string &type)
    : BaseNode(type, "Print Node")
{
    m_label = GenerateRandomString(10, BaseNode::CHARSET_ALPHABET_LOWER | BaseNode::CHARSET_ALPHABET_UPPER );// Should be enough to avoid collision?

    // Create empty variable in memory
    auto v = std::make_shared<Variable>(m_label);
    v->SetTextValue("");
    m_variables[m_label] = v;
}


void PrintNode::Initialize()
{

}

std::string PrintNode::Build(IStoryPage &page, const StoryOptions &options, int nb_out_conns)
{
    return "";
}


