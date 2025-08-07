#pragma once

#include <string>

class ILogSubject {
public:
    virtual void LogEvent(const std::string &txt, bool critical = false) = 0;
};

class ILogger
{

public:
    virtual void Log(const std::string &txt, bool critical = false)  = 0;
    virtual void RegisterSubject(std::shared_ptr<ILogSubject> subject) = 0;
    virtual ~ILogger() {}
};
