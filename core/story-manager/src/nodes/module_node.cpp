#include "module_node.h"
#include "story_project.h"
#include "connection.h"
#include "sys_lib.h"


ModuleNode::ModuleNode(const std::string &type)
    : BaseNode(type, "Module Node")
{
    nlohmann::json j{ {"module", ""} };
    SetInternalData(j);
}

void ModuleNode::Initialize()
{
    nlohmann::json j = GetInternalData();
    // m_image = j["image"].get<std::string>();
    // m_sound = j["sound"].get<std::string>();
}

