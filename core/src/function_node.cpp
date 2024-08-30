#include "function_node.h"
#include "story_project.h"
#include "connection.h"
#include "sys_lib.h"


FunctionNode::FunctionNode(const std::string &type)
    : BaseNode(type, "Function Node")
{
    nlohmann::json j{ {"function", ""} };
    SetInternalData(j);
}

void FunctionNode::StoreInternalData()
{
    nlohmann::json j;
    // j["image"] = m_image;
    // j["sound"] = m_sound;

    SetInternalData(j);
}

void FunctionNode::Initialize()
{
    nlohmann::json j = GetInternalData();
    // m_image = j["image"].get<std::string>();
    // m_sound = j["sound"].get<std::string>();
}

std::string FunctionNode::Build(IStoryPage &page, const StoryOptions &options, int nb_out_conns)
{
    return std::string();
}

std::string FunctionNode::GenerateConstants(IStoryPage &page, const StoryOptions &options, int nb_out_conns)
{
    std::string s;



    return s;
}

