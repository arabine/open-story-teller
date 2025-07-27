#pragma once

#include <list>
#include <string>
#include "connection.h"
#include "story_options.h"

class IStoryProject
{
public:

    enum Type
    {
        PROJECT_TYPE_STORY = 0,
        PROJECT_TYPE_MODULE = 1,
    };

    virtual ~IStoryProject() {};

    virtual std::string GetName() const = 0;

};



