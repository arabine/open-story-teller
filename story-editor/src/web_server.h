#pragma once

#include "CivetServer.h"
#include "library_manager.h"

class HandlerBase : public CivetHandler
{
public:

protected:
    // Utility methods for children

    bool Reply(struct mg_connection *conn, const nlohmann::json &json);
};

class LibraryManagerHandler : public HandlerBase
{
public:
    LibraryManagerHandler(LibraryManager &libraryManager)
        : m_libraryManager(libraryManager)
    {

    }

    bool handleGet(CivetServer *server, struct mg_connection *conn);

	bool
	handlePost(CivetServer *server, struct mg_connection *conn)
	{
		/* Handler may access the request info using mg_get_request_info */
		const struct mg_request_info *req_info = mg_get_request_info(conn);
		long long rlen, wlen;
		long long nlen = 0;
		long long tlen = req_info->content_length;
		char buf[1024];

		mg_printf(conn,
		          "HTTP/1.1 200 OK\r\nContent-Type: "
		          "text/html\r\nConnection: close\r\n\r\n");

		mg_printf(conn, "<html><body>\n");
		mg_printf(conn, "<h2>This is the Foo POST handler!!!</h2>\n");
		mg_printf(conn,
		          "<p>The request was:<br><pre>%s %s HTTP/%s</pre></p>\n",
		          req_info->request_method,
		          req_info->request_uri,
		          req_info->http_version);
		mg_printf(conn, "<p>Content Length: %li</p>\n", (long)tlen);
		mg_printf(conn, "<pre>\n");

		while (nlen < tlen) {
			rlen = tlen - nlen;
			if (rlen > sizeof(buf)) {
				rlen = sizeof(buf);
			}
			rlen = mg_read(conn, buf, (size_t)rlen);
			if (rlen <= 0) {
				break;
			}
			wlen = mg_write(conn, buf, (size_t)rlen);
			if (wlen != rlen) {
				break;
			}
			nlen += wlen;
		}

		mg_printf(conn, "\n</pre>\n");
		mg_printf(conn, "</body></html>\n");

		return true;
	}

    #define fopen_recursive fopen

    bool
        handlePut(CivetServer *server, struct mg_connection *conn)
    {
        /* Handler may access the request info using mg_get_request_info */
        const struct mg_request_info *req_info = mg_get_request_info(conn);
        long long rlen, wlen;
        long long nlen = 0;
        long long tlen = req_info->content_length;
        FILE * f;
        char buf[1024];
        int fail = 0;

#ifdef _WIN32
        _snprintf(buf, sizeof(buf), "D:\\somewhere\\%s\\%s", req_info->remote_user, req_info->local_uri);
        buf[sizeof(buf)-1] = 0;
        if (strlen(buf)>255) {
            /* Windows will not work with path > 260 (MAX_PATH), unless we use
             * the unicode API. However, this is just an example code: A real
             * code will probably never store anything to D:\\somewhere and
             * must be adapted to the specific needs anyhow. */
            fail = 1;
            f = NULL;
        } else {
            f = fopen_recursive(buf, "wb");
        }
#else
        snprintf(buf, sizeof(buf), "~/somewhere/%s/%s", req_info->remote_user, req_info->local_uri);
        buf[sizeof(buf)-1] = 0;
        if (strlen(buf)>1020) {
            /* The string is too long and probably truncated. Make sure an
             * UTF-8 string is never truncated between the UTF-8 code bytes.
             * This example code must be adapted to the specific needs. */
            fail = 1;
            f = NULL;
        } else {
            f = fopen_recursive(buf, "w");
        }
#endif

        if (!f) {
            fail = 1;
        } else {
            while (nlen < tlen) {
                rlen = tlen - nlen;
                if (rlen > sizeof(buf)) {
                    rlen = sizeof(buf);
                }
                rlen = mg_read(conn, buf, (size_t)rlen);
                if (rlen <= 0) {
                    fail = 1;
                    break;
                }
                wlen = fwrite(buf, 1, (size_t)rlen, f);
                if (wlen != rlen) {
                    fail = 1;
                    break;
                }
                nlen += wlen;
            }
            fclose(f);
        }

        if (fail) {
            mg_printf(conn,
                "HTTP/1.1 409 Conflict\r\n"
                "Content-Type: text/plain\r\n"
                "Connection: close\r\n\r\n");
        } else {
            mg_printf(conn,
                "HTTP/1.1 201 Created\r\n"
                "Content-Type: text/plain\r\n"
                "Connection: close\r\n\r\n");
        }

        return true;
    }

private:
    LibraryManager &m_libraryManager;
};

class WebServer :  public CivetHandler
{

public:
    WebServer(LibraryManager &libraryManager);
    ~WebServer();

private:
    LibraryManager &m_libraryManager;
    
    CivetServer m_server;
    LibraryManagerHandler m_libraryManagerHandler;

};
