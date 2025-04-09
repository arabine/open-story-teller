#include "variable_node.h"
#include "story_project.h"
#include "connection.h"
#include "sys_lib.h"


VariableNode::VariableNode(const std::string &type)
    : BaseNode(type, "Variable Node")
{
    nlohmann::json j{ {"name", "i"}, {"value", 3} };
    SetInternalData(j);
}

void VariableNode::StoreInternalData()
{
    nlohmann::json j;
    // j["image"] = m_image;
    // j["sound"] = m_sound;

    SetInternalData(j);
}

void VariableNode::Initialize()
{
    nlohmann::json j = GetInternalData();
    // m_image = j["image"].get<std::string>();
    // m_sound = j["sound"].get<std::string>();
}

std::string VariableNode::Build(IStoryPage &page, const StoryOptions &options, int nb_out_conns)
{
    return std::string();
}

std::string VariableNode::GenerateConstants(IStoryPage &page, IStoryProject &project, int nb_out_conns)
{
    std::string s;



    return s;
}

