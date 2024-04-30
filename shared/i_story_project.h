#pragma once

#include <list>
#include <string>
#include "connection.h"

class IStoryProject
{
public:
    virtual ~IStoryProject() {};
    virtual std::list<std::shared_ptr<Connection>> GetNodeConnections(const std::string &nodeId) = 0;
    virtual int OutputsCount(const std::string &nodeId) = 0;
};



