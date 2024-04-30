#pragma once

#include <string>
#include "i_story_manager.h"
#include "base_node.h"
#include "i_script_node.h"
#include "i_story_project.h"

struct MediaNode : public BaseNode
{

    MediaNode(const std::string &type);

    virtual std::string Build(IStoryProject &story, int nb_out_conns) override;
    virtual std::string GenerateConstants(IStoryProject &story, int nb_out_conns) override;

    std::string image;
    std::string sound;
};

