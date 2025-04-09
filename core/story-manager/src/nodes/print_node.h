#pragma once

#include <string>
#include "i_story_manager.h"
#include "execution_node.h"
#include "i_script_node.h"
#include "i_story_project.h"

class PrintNode : public ExecutionNode
{
public:
    PrintNode(const std::string &type);

    virtual void Initialize() override;
    virtual std::string Build(IStoryPage &page, const StoryOptions &options, int nb_out_conns) override;
    virtual std::string GenerateConstants(IStoryPage &page, IStoryProject &project, int nb_out_conns) override;

    virtual std::string GenerateConstants() const override;

    std::string GenerateAssembly() const override;

    void SetText(const std::string &text) {
        m_text = text;
    }

    std::string GetText() const {
        return m_text;
    }

private:
    std::string m_label;
    std::string m_text; // Text to print
    uint32_t m_arguments{0}; // number of arguments
};

