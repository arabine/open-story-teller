#include "print_node.h"
#include "story_project.h"
#include "connection.h"
#include "sys_lib.h"
#include "compiler.h"


PrintNode::PrintNode(const std::string &type)
    : BaseNode(type, "Print Node")
{

}


void PrintNode::Initialize()
{

}

std::string PrintNode::GenerateConstants(IStoryPage &page, IStoryProject &project, int nb_out_conns)
{
    std::string s;

   

    return s;
}

std::string PrintNode::Build(IStoryPage &page, const StoryOptions &options, int nb_out_conns)
{
    std::stringstream ss;

    std::list<std::shared_ptr<Connection>> conns;
    page.GetNodeConnections(conns, GetId());
    int i = 0;
    std::list<std::shared_ptr<Connection>>::iterator c = conns.begin();

    if (conns.size() == 2)
    {
        ss << R"(; ---------------------------- )"
        << GetTitle()
        << " Type: Branch"
        << "\n";

        ss << "eq  r0, r0, r1\n"
           << "skipz r0\n"
           << "jump " << BaseNode::GetEntryLabel((*c)->inNodeId);
        ++c;
        ss << "jump " << BaseNode::GetEntryLabel((*c)->inNodeId);
    }
    return ss.str();
}

