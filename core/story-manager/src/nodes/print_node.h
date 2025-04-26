#pragma once

#include <string>
#include "i_story_manager.h"
#include "base_node.h"
#include "i_script_node.h"
#include "i_story_project.h"

class PrintNode : public BaseNode
{
public:
    PrintNode(const std::string &type);

    virtual void Initialize() override;

    void SetText(const std::string &text);
    std::string GetLabel() const;
    std::string GetText() const;

private:
    std::string m_label; // Label for the string literal
    uint32_t m_arguments{0}; // number of arguments
};

