#pragma once

#include <string>
#include "i_story_manager.h"
#include "base_node.h"
#include "i_script_node.h"
#include "i_story_project.h"

class ModuleNode : public BaseNode
{
public:
    ModuleNode(const std::string &type);

    virtual void Initialize() override;

    void SetContent(nlohmann::json &content) {
        m_content = content;
    }

private:
    nlohmann::json m_content;
};

