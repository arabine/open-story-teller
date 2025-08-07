#pragma once

#include <string>
#include <vector>
#include <memory>
#include "i_logger.h"

class Logger : public ILogger
{
public:
    void Log(const std::string& message, bool critical = false) override
    {
        for (const auto& subject : m_subjects)
        {
            if (subject)
            {
                subject->LogEvent(message, critical);
            }
        }
    }

    void RegisterSubject(std::shared_ptr<ILogSubject> subject) override
    {
        m_subjects.push_back(subject);
    }

private:
    // List of log subjects
    std::vector<std::shared_ptr<ILogSubject>> m_subjects;
};

