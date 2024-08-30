#pragma once

#include "sys_lib.h"

class Compiler
{

public:
    Compiler() = default;
    ~Compiler() = default;

    static std::string FileToConstant(const std::string &FileName, const std::string &extension);


};
