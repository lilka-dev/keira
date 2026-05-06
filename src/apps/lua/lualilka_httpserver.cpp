#include "lualilka_httpserver.h"

#include <WString.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>
#include <cerrno>

#define HTTPSERVER_MAX_HEADER_BYTES 8192
#define HTTPSERVER_MAX_BODY_BYTES   65536
#define HTTPSERVER_CLIENT_TIMEOUT_S 10
#define HTTPSERVER_CHUNK_SIZE       1024

// httpserver.listen(port [, backlog]) -> fd | nil, errmsg
static int lualilka_httpserver_listen(lua_State* L) {
    int port = (int)luaL_checkinteger(L, 1);
    int backlog = (int)luaL_optinteger(L, 2, 5);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        lua_pushnil(L);
        lua_pushstring(L, "socket() failed");
        return 2;
    }

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((uint16_t)port);

    if (bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) != 0) {
        close(fd);
        lua_pushnil(L);
        lua_pushfstring(L, "bind() failed: errno %d", errno);
        return 2;
    }

    if (listen(fd, backlog) != 0) {
        close(fd);
        lua_pushnil(L);
        lua_pushfstring(L, "listen() failed: errno %d", errno);
        return 2;
    }

    lua_pushinteger(L, fd);
    return 1;
}

// Reads HTTP headers in chunks until \r\n\r\n is found.
// Returns false on error. `headers` contains everything up to and including \r\n\r\n.
// `body_prefix` contains any bytes received past \r\n\r\n in the last chunk.
static bool recv_http_headers(int fd, String& headers, String& body_prefix) {
    char chunk[HTTPSERVER_CHUNK_SIZE];
    while (headers.length() <= HTTPSERVER_MAX_HEADER_BYTES) {
        ssize_t n = recv(fd, chunk, sizeof(chunk), 0);
        if (n <= 0) return false;
        headers.concat(chunk, (unsigned int)n);
        int idx = headers.indexOf("\r\n\r\n");
        if (idx >= 0) {
            body_prefix = headers.substring((unsigned int)(idx + 4));
            headers = headers.substring(0, (unsigned int)(idx + 4));
            return true;
        }
    }
    return false;
}

// httpserver.accept(server_fd [, timeout_ms]) -> request | nil, errmsg
// request = { fd, client_ip, method, path, query, headers={}, body }
static int lualilka_httpserver_accept(lua_State* L) {
    int server_fd = (int)luaL_checkinteger(L, 1);
    int timeout_ms = (int)luaL_optinteger(L, 2, -1);

    if (timeout_ms >= 0) {
        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    }

    struct sockaddr_in client_addr = {};
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(server_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &addr_len);
    if (client_fd < 0) {
        int saved_errno = errno;
        lua_pushnil(L);
        if (saved_errno == EAGAIN || saved_errno == EWOULDBLOCK) {
            lua_pushstring(L, "timeout");
        } else {
            lua_pushfstring(L, "accept() failed: errno %d", saved_errno);
        }
        return 2;
    }

    // Apply I/O timeout on client connection
    struct timeval io_tv;
    io_tv.tv_sec = HTTPSERVER_CLIENT_TIMEOUT_S;
    io_tv.tv_usec = 0;
    setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &io_tv, sizeof(io_tv));
    setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, &io_tv, sizeof(io_tv));

    // Read headers in chunks
    String raw_headers, body_prefix;
    if (!recv_http_headers(client_fd, raw_headers, body_prefix)) {
        close(client_fd);
        lua_pushnil(L);
        lua_pushstring(L, "failed to read request headers");
        return 2;
    }

    // Parse request line: METHOD SP path SP HTTP/x.y\r\n
    int crlf1 = raw_headers.indexOf("\r\n");
    if (crlf1 < 0) {
        close(client_fd);
        lua_pushnil(L);
        lua_pushstring(L, "malformed request line");
        return 2;
    }
    String request_line = raw_headers.substring(0, (unsigned int)crlf1);
    int sp1 = request_line.indexOf(' ');
    int sp2 = request_line.lastIndexOf(' ');
    if (sp1 < 0 || sp2 <= sp1) {
        close(client_fd);
        lua_pushnil(L);
        lua_pushstring(L, "malformed request line");
        return 2;
    }
    String method    = request_line.substring(0, (unsigned int)sp1);
    String full_path = request_line.substring((unsigned int)(sp1 + 1), (unsigned int)sp2);

    // Split path and query string
    String path, query;
    int qmark = full_path.indexOf('?');
    if (qmark >= 0) {
        path  = full_path.substring(0, (unsigned int)qmark);
        query = full_path.substring((unsigned int)(qmark + 1));
    } else {
        path = full_path;
    }

    // Get client IP
    char client_ip[INET_ADDRSTRLEN] = {};
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

    // Build result table
    lua_createtable(L, 0, 7);

    lua_pushinteger(L, client_fd);
    lua_setfield(L, -2, "fd");

    lua_pushstring(L, client_ip);
    lua_setfield(L, -2, "client_ip");

    lua_pushstring(L, method.c_str());
    lua_setfield(L, -2, "method");

    lua_pushstring(L, path.c_str());
    lua_setfield(L, -2, "path");

    lua_pushstring(L, query.c_str());
    lua_setfield(L, -2, "query");

    // Parse headers into sub-table (keys lowercased)
    lua_createtable(L, 0, 8);
    int content_length = 0;
    int hdr_pos = crlf1 + 2;
    while (hdr_pos < (int)raw_headers.length()) {
        int eol = raw_headers.indexOf("\r\n", (unsigned int)hdr_pos);
        if (eol < 0 || eol == hdr_pos) break;
        String hdr_line = raw_headers.substring((unsigned int)hdr_pos, (unsigned int)eol);
        int colon = hdr_line.indexOf(':');
        if (colon > 0) {
            String name  = hdr_line.substring(0, (unsigned int)colon);
            String value = hdr_line.substring((unsigned int)(colon + 1));
            name.toLowerCase();
            value.trim();
            lua_pushstring(L, value.c_str());
            lua_setfield(L, -2, name.c_str());
            if (name == "content-length") {
                content_length = value.toInt();
            }
        }
        hdr_pos = eol + 2;
    }
    lua_setfield(L, -2, "headers");

    // Read body if Content-Length is present and within limit
    if (content_length > 0 && content_length <= HTTPSERVER_MAX_BODY_BYTES) {
        String body = body_prefix;
        char body_chunk[HTTPSERVER_CHUNK_SIZE];
        while ((int)body.length() < content_length) {
            ssize_t n = recv(client_fd, body_chunk, sizeof(body_chunk), 0);
            if (n <= 0) break;
            body.concat(body_chunk, (unsigned int)n);
        }
        if ((int)body.length() > content_length) {
            body = body.substring(0, (unsigned int)content_length);
        }
        lua_pushlstring(L, body.c_str(), body.length());
    } else {
        lua_pushstring(L, "");
    }
    lua_setfield(L, -2, "body");

    return 1;
}

// httpserver.respond(fd, status [, headers_table [, body]])
// Sends a complete HTTP/1.1 response and closes the connection.
static int lualilka_httpserver_respond(lua_State* L) {
    int fd = (int)luaL_checkinteger(L, 1);
    int status = (int)luaL_checkinteger(L, 2);

    const char* status_text = "OK";
    if (status == 201) status_text = "Created";
    else if (status == 204) status_text = "No Content";
    else if (status == 301) status_text = "Moved Permanently";
    else if (status == 302) status_text = "Found";
    else if (status == 400) status_text = "Bad Request";
    else if (status == 401) status_text = "Unauthorized";
    else if (status == 403) status_text = "Forbidden";
    else if (status == 404) status_text = "Not Found";
    else if (status == 405) status_text = "Method Not Allowed";
    else if (status == 500) status_text = "Internal Server Error";

    size_t body_len = 0;
    const char* body = nullptr;
    if (lua_isstring(L, 4)) {
        body = lua_tolstring(L, 4, &body_len);
    }

    // Build response header
    String head = "HTTP/1.1 ";
    head += status;
    head += " ";
    head += status_text;
    head += "\r\n";

    // Custom headers from table argument
    if (lua_istable(L, 3)) {
        lua_pushnil(L);
        while (lua_next(L, 3) != 0) {
            if (lua_isstring(L, -2) && lua_isstring(L, -1)) {
                head += lua_tostring(L, -2);
                head += ": ";
                head += lua_tostring(L, -1);
                head += "\r\n";
            }
            lua_pop(L, 1);
        }
    }

    head += "Content-Length: ";
    head += (int)body_len;
    head += "\r\nConnection: close\r\n\r\n";

    send(fd, head.c_str(), head.length(), 0);

    if (body && body_len > 0) {
        send(fd, body, body_len, 0);
    }

    close(fd);
    return 0;
}

// httpserver.close(fd)
static int lualilka_httpserver_close(lua_State* L) {
    int fd = (int)luaL_checkinteger(L, 1);
    close(fd);
    return 0;
}

static const struct luaL_Reg lualilka_httpserver_funcs[] = {
    {"listen", lualilka_httpserver_listen},
    {"accept", lualilka_httpserver_accept},
    {"respond", lualilka_httpserver_respond},
    {"close", lualilka_httpserver_close},
    {NULL, NULL},
};

int lualilka_httpserver_register(lua_State* L) {
    luaL_newlib(L, lualilka_httpserver_funcs);
    lua_setglobal(L, "httpserver");
    return 0;
}
