#pragma once

#include "i_story_project.h"


class StoryPrimitive : public IStoryProject
{

public:
    StoryPrimitive(const std::string &name, const std::string &description = "", const std::string &uuid = "00000000-0000-0000-0000-000000000000")
        : m_name(name)
        , m_description(description)
        , m_uuid(uuid)
    {

    }
    virtual ~StoryPrimitive() {};

    virtual std::string GetName() const
    {
        return m_name;
    }

    virtual std::string GetDescription() const
    {
        return m_description;
    }

    virtual bool IsModule() const
    {
        return false; // This is not a module, it's a primitive
    }

    virtual std::string GetUuid() const
    {
        return m_uuid;
    }

private:
    std::string m_name;
    std::string m_description;
    std::string m_uuid;
};