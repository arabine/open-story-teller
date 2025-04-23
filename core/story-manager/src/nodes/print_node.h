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

    void SetText(const std::string &text) {

        m_variables.at(m_label)->SetValue<std::string>(text);
    }

    std::string GetLabel() const {
        return m_label;
    }

    std::string GetText() const {
        return m_variables.at(m_label)->GetValue<std::string>();
    }

private:
    std::string m_label; // Label for the string literal
    uint32_t m_arguments{0}; // number of arguments
};

