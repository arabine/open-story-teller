
#include <string.h>
#include "json.hpp"

#include "web_server.h"

#define DOCUMENT_ROOT "."
#define PORT "8081"

bool HandlerBase::Reply(struct mg_connection *conn, const nlohmann::json &json)
{
    std::stringstream ss;

    std::string data = json.dump();

    /* Send HTTP message header (+1 for \n) */
	mg_send_http_ok(conn, "application/json; charset=utf-8", data.size() + 1);

	/* Send HTTP message content */
	mg_write(conn, data.c_str(), data.size());

	/* Add a newline. This is not required, but the result is more
	 * human-readable in a debugger. */
	mg_write(conn, "\n", 1);
    return true;
}

bool LibraryManagerHandler::handleGet(CivetServer *server, struct mg_connection *conn)
{
    const struct mg_request_info *req_info = mg_get_request_info(conn);
    nlohmann::json json;

    for (auto &s : m_libraryManager)
    {
        nlohmann::json story = {
            {"title", s->GetName() },
            {"uuid", s->GetUuid() },
        };
        json.push_back(story);
        
    }
    return Reply(conn, json);
}


static const char *options[] = {
    "document_root", DOCUMENT_ROOT, 
    "listening_ports", PORT, 
    "access_control_allow_origin", "*",
    "access_control_allow_methods", "GET, POST, PUT, DELETE, OPTIONS",
    "access_control_allow_headers", "Content-Type",
    0
};

static const std::string gRestBase = "/api/v1";

WebServer::WebServer(LibraryManager &libraryManager)
    : m_libraryManager(libraryManager)
    , m_server(options)
    , m_libraryManagerHandler(libraryManager)
{
    mg_init_library(0);
	
	m_server.addHandler(gRestBase + "/library/list", m_libraryManagerHandler);
//	printf("Browse files at http://localhost:%s/\n", PORT);
}

WebServer::~WebServer()
{
    mg_exit_library();
}

