#pragma once

#include <list>
#include <string>
#include "connection.h"
#include "story_options.h"

class IStoryProject
{
public:
    virtual ~IStoryProject() {};

    virtual std::list<std::shared_ptr<Connection>> GetNodeConnections(const std::string &nodeId) = 0;
    virtual int OutputsCount(const std::string &nodeId) = 0;
    virtual StoryOptions GetOptions() = 0;

    /* Retourne true si la resource existe déjà et que le code a déjà été généré */
    virtual bool UseResource(const std::string &label) = 0;
};



