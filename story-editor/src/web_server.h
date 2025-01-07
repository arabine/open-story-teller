#pragma once

#include "CivetServer.h"

class WebServer :  public CivetHandler
{

public:
    WebServer();
    ~WebServer();

private:

    CivetServer m_server;

};
