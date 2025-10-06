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
    
    static constexpr int MAX_INPUT_COUNT = 4;

    std::shared_ptr<Variable> GetVariable(const std::string& label) const {
        auto it = m_variables.find(label);
        if (it != m_variables.end()) {
            return it->second;
        }
        return nullptr;
    }

private:
    std::string m_label; // Label for the string literal
};