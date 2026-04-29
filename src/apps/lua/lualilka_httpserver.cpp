#include "lualilka_httpserver.h"

#include <lwip/sockets.h>
#include <lwip/inet.h>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#define HTTPSERVER_MAX_HEADER_BYTES 8192
#define HTTPSERVER_MAX_BODY_BYTES   65536
#define HTTPSERVER_CLIENT_TIMEOUT_S 10

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

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
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

// Read until \r\n\r\n; returns malloc'd buffer (caller frees) or NULL on error.
static char* recv_http_headers(int fd, size_t* out_len) {
    size_t capacity = 1024;
    size_t len = 0;
    char* buf = (char*)malloc(capacity);
    if (!buf) return nullptr;

    while (len < HTTPSERVER_MAX_HEADER_BYTES) {
        if (len + 2 >= capacity) {
            size_t new_cap = capacity * 2;
            if (new_cap > HTTPSERVER_MAX_HEADER_BYTES + 4) {
                new_cap = HTTPSERVER_MAX_HEADER_BYTES + 4;
            }
            char* nb = (char*)realloc(buf, new_cap);
            if (!nb) { free(buf); return nullptr; }
            buf = nb;
            capacity = new_cap;
        }

        ssize_t n = recv(fd, buf + len, 1, 0);
        if (n <= 0) { free(buf); return nullptr; }
        len++;
        buf[len] = '\0';

        if (len >= 4 && memcmp(buf + len - 4, "\r\n\r\n", 4) == 0) {
            *out_len = len;
            return buf;
        }
    }

    free(buf);
    return nullptr; // header too large
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
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
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

    // Read headers
    size_t hdr_len;
    char* hdr_buf = recv_http_headers(client_fd, &hdr_len);
    if (!hdr_buf) {
        close(client_fd);
        lua_pushnil(L);
        lua_pushstring(L, "failed to read request headers");
        return 2;
    }

    // Parse request line: METHOD SP path SP HTTP/x.y\r\n
    char* line_end = strstr(hdr_buf, "\r\n");
    if (!line_end) {
        free(hdr_buf);
        close(client_fd);
        lua_pushnil(L);
        lua_pushstring(L, "malformed request line");
        return 2;
    }

    *line_end = '\0';
    char method[16] = {};
    char full_path[512] = {};
    sscanf(hdr_buf, "%15s %511s", method, full_path);
    *line_end = '\r';

    // Split path and query string
    char path[512] = {};
    char query[512] = {};
    char* qmark = strchr(full_path, '?');
    if (qmark) {
        size_t plen = (size_t)(qmark - full_path);
        if (plen >= sizeof(path)) plen = sizeof(path) - 1;
        memcpy(path, full_path, plen);
        strncpy(query, qmark + 1, sizeof(query) - 1);
    } else {
        strncpy(path, full_path, sizeof(path) - 1);
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

    lua_pushstring(L, method);
    lua_setfield(L, -2, "method");

    lua_pushstring(L, path);
    lua_setfield(L, -2, "path");

    lua_pushstring(L, query);
    lua_setfield(L, -2, "query");

    // Parse headers into sub-table (keys lowercased)
    lua_createtable(L, 0, 8);
    int content_length = 0;
    char* cursor = line_end + 2; // skip \r\n after request line
    while (*cursor && !(*cursor == '\r' && *(cursor + 1) == '\n')) {
        char* hdr_eol = strstr(cursor, "\r\n");
        if (!hdr_eol) break;
        *hdr_eol = '\0';

        char* colon = strchr(cursor, ':');
        if (colon) {
            *colon = '\0';
            char* val = colon + 1;
            while (*val == ' ' || *val == '\t') val++;

            // Lowercase the header name in-place
            for (char* p = cursor; *p; p++) {
                if (*p >= 'A' && *p <= 'Z') *p += 32;
            }

            lua_pushstring(L, val);
            lua_setfield(L, -2, cursor);

            if (strcmp(cursor, "content-length") == 0) {
                content_length = atoi(val);
            }

            *colon = ':';
        }

        *hdr_eol = '\r';
        cursor = hdr_eol + 2;
    }
    lua_setfield(L, -2, "headers");

    free(hdr_buf);

    // Read body if Content-Length is set and within limit
    if (content_length > 0 && content_length <= HTTPSERVER_MAX_BODY_BYTES) {
        char* body = (char*)malloc((size_t)content_length + 1);
        if (body) {
            int received = 0;
            while (received < content_length) {
                ssize_t n = recv(client_fd, body + received, (size_t)(content_length - received), 0);
                if (n <= 0) break;
                received += (int)n;
            }
            body[received] = '\0';
            lua_pushlstring(L, body, (size_t)received);
            free(body);
        } else {
            lua_pushstring(L, "");
        }
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

    // Build response header into a heap buffer to avoid stack overflow
    size_t head_cap = 2048;
    char* head = (char*)malloc(head_cap);
    if (!head) {
        close(fd);
        return luaL_error(L, "out of memory");
    }

    int head_len = snprintf(head, head_cap, "HTTP/1.1 %d %s\r\n", status, status_text);

    // Custom headers from table argument
    if (lua_istable(L, 3)) {
        lua_pushnil(L);
        while (lua_next(L, 3) != 0) {
            if (lua_isstring(L, -2) && lua_isstring(L, -1)) {
                const char* k = lua_tostring(L, -2);
                const char* v = lua_tostring(L, -1);
                int written = snprintf(head + head_len, head_cap - (size_t)head_len, "%s: %s\r\n", k, v);
                if (written > 0) head_len += written;
            }
            lua_pop(L, 1);
        }
    }

    int written = snprintf(
        head + head_len, head_cap - (size_t)head_len,
        "Content-Length: %d\r\nConnection: close\r\n\r\n", (int)body_len
    );
    if (written > 0) head_len += written;

    send(fd, head, (size_t)head_len, 0);
    free(head);

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
