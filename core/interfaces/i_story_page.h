#pragma once

#include <list>
#include "connection.h"

class IStoryPage
{
public:
    virtual ~IStoryPage() {}

     virtual void GetNodeConnections(std::list<std::shared_ptr<Connection>> &c, const std::string &nodeId) = 0;
};
