#pragma once

#include "i_story_project.h"


class StoryPrimitive : public IStoryProject
{

public:
    StoryPrimitive(const std::string &name)
        : m_name(name)
    {

    }
    virtual ~StoryPrimitive() {};

    virtual std::string GetName() const
    {
        return m_name;
    }

private:
    std::string m_name;

};