#pragma once

#include <string>
#include "i_story_manager.h"
#include "base_node.h"
#include "i_script_node.h"
#include "i_story_project.h"

class BranchNode : public BaseNode
{
public:
    BranchNode(const std::string &type);

    virtual void Initialize() override;
    virtual std::string Build(IStoryPage &page, const StoryOptions &options, int nb_out_conns) override;
    virtual std::string GenerateConstants(IStoryPage &page, IStoryProject &project, int nb_out_conns) override;

    virtual std::string GenerateAssembly() const { return ""; }

private:
};

