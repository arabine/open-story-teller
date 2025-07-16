#include "call_function_node.h"
#include "story_project.h"
#include "connection.h"
#include "sys_lib.h"


CallFunctionNode::CallFunctionNode(const std::string &type)
    : BaseNode(type, "Call Function Node")
{
    nlohmann::json j{ {"function", ""} };
    SetInternalData(j);
}

void CallFunctionNode::Initialize()
{
    nlohmann::json j = GetInternalData();
    // m_image = j["image"].get<std::string>();
    // m_sound = j["sound"].get<std::string>();
}

