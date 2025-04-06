#include "print_node.h"
#include "story_project.h"
#include "connection.h"
#include "sys_lib.h"
#include "compiler.h"


PrintNode::PrintNode(const std::string &type)
    : ExecutionNode(type, "Print Node")
{
    m_label = GenerateRandomString(10, BaseNode::CHARSET_ALPHABET_LOWER | BaseNode::CHARSET_ALPHABET_UPPER );// Should be enough to avoid collision?
}


void PrintNode::Initialize()
{

}

std::string PrintNode::GenerateConstants(IStoryPage &page, IStoryProject &project, int nb_out_conns)
{
    std::stringstream ss;
    ss  << "$" << m_label << " DC8, "  << m_text << "\n";
    return ss.str();
}

std::string PrintNode::Build(IStoryPage &page, const StoryOptions &options, int nb_out_conns)
{
    return "";
}

std::string PrintNode::GenerateConstants() const
{
    std::stringstream ss;
    ss  << "$" << m_label << " DC8, \""  << m_text << "\"\n";
    return ss.str();
}

std::string PrintNode::GenerateAssembly() const
{

    std::stringstream ss;

    ss  << ExecutionNode::GenerateAssembly()
        << "  push r0\n"
        << "  push r1\n"
        << "  lcons r0, $" << m_label << "\n"
        << "  lcons r1, 0 ; number of arguments\n"  // FIXME: handle arguments
        << "  syscall 4\n"
        << "  pop r1\n"
        << "  pop r0\n";

//        << ""mov r2, %2 // arguments are in r2, r3, r4 etc.
    
      return ss.str();
}

