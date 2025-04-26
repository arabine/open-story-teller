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

void FunctionNode::Initialize()
{
    nlohmann::json j = GetInternalData();
    // m_image = j["image"].get<std::string>();
    // m_sound = j["sound"].get<std::string>();
}

