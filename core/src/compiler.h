#pragma once

#include "sys_lib.h"
#include "i_story_project.h"

class Compiler
{

public:
    Compiler() = default;
    ~Compiler() = default;

    static std::string FileToConstant(const std::string &FileName, const std::string &extension, IStoryProject &project);


};
