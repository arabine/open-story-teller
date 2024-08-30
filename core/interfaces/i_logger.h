#pragma once

#include <string>

class ILogger
{

public:
    virtual void Log(const std::string &txt, bool critical = false)  = 0;
    virtual ~ILogger() {}
};
