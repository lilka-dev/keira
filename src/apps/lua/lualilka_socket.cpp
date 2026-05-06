#include "lualilka_socket.h"

#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <lwip/inet.h>
#include <cerrno>

// net.connect(host, port [, timeout_ms]) -> fd | nil, errmsg
static int lualilka_net_connect(lua_State* L) {
    const char* host = luaL_checkstring(L, 1);
    int port = (int)luaL_checkinteger(L, 2);
    int timeout_ms = (int)luaL_optinteger(L, 3, 5000);

    char port_str[8];
    snprintf(port_str, sizeof(port_str), "%d", port);

    struct addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* res = nullptr;
    if (getaddrinfo(host, port_str, &hints, &res) != 0 || res == nullptr) {
        lua_pushnil(L);
        lua_pushfstring(L, "DNS lookup failed for %s", host);
        return 2;
    }

    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) {
        freeaddrinfo(res);
        lua_pushnil(L);
        lua_pushstring(L, "socket() failed");
        return 2;
    }

    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    if (connect(fd, res->ai_addr, res->ai_addrlen) != 0) {
        int saved_errno = errno;
        freeaddrinfo(res);
        close(fd);
        lua_pushnil(L);
        lua_pushfstring(L, "connect() failed: errno %d", saved_errno);
        return 2;
    }

    freeaddrinfo(res);
    lua_pushinteger(L, fd);
    return 1;
}

// net.send(fd, data) -> bytes_sent | nil, errmsg
static int lualilka_net_send(lua_State* L) {
    int fd = (int)luaL_checkinteger(L, 1);
    size_t len;
    const char* data = luaL_checklstring(L, 2, &len);

    ssize_t sent = send(fd, data, len, 0);
    if (sent < 0) {
        lua_pushnil(L);
        lua_pushfstring(L, "send() failed: errno %d", errno);
        return 2;
    }
    lua_pushinteger(L, (lua_Integer)sent);
    return 1;
}

// net.receive(fd [, max_bytes [, timeout_ms]]) -> data | nil, errmsg
static int lualilka_net_receive(lua_State* L) {
    int fd = (int)luaL_checkinteger(L, 1);
    int max_bytes = (int)luaL_optinteger(L, 2, 1024);
    int timeout_ms = (int)luaL_optinteger(L, 3, -1);

    if (timeout_ms >= 0) {
        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    }

    char* buf = static_cast<char*>(malloc((size_t)max_bytes));
    if (!buf) {
        lua_pushnil(L);
        lua_pushstring(L, "out of memory");
        return 2;
    }

    ssize_t n = recv(fd, buf, (size_t)max_bytes, 0);
    if (n < 0) {
        int saved_errno = errno;
        free(buf);
        if (saved_errno == EAGAIN || saved_errno == EWOULDBLOCK) {
            lua_pushnil(L);
            lua_pushstring(L, "timeout");
            return 2;
        }
        lua_pushnil(L);
        lua_pushfstring(L, "recv() failed: errno %d", saved_errno);
        return 2;
    }
    if (n == 0) {
        free(buf);
        lua_pushnil(L);
        lua_pushstring(L, "connection closed");
        return 2;
    }

    lua_pushlstring(L, buf, (size_t)n);
    free(buf);
    return 1;
}

// net.close(fd)
static int lualilka_net_close(lua_State* L) {
    int fd = (int)luaL_checkinteger(L, 1);
    close(fd);
    return 0;
}

// net.listen(port [, backlog]) -> fd | nil, errmsg
static int lualilka_net_listen(lua_State* L) {
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

// net.accept(server_fd [, timeout_ms]) -> client_fd, client_ip | nil, errmsg
static int lualilka_net_accept(lua_State* L) {
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

    char ip_str[INET_ADDRSTRLEN] = {};
    inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));

    lua_pushinteger(L, client_fd);
    lua_pushstring(L, ip_str);
    return 2;
}

// net.settimeout(fd, timeout_ms)
static int lualilka_net_settimeout(lua_State* L) {
    int fd = (int)luaL_checkinteger(L, 1);
    int timeout_ms = (int)luaL_checkinteger(L, 2);

    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    return 0;
}

static const struct luaL_Reg lualilka_net[] = {
    {"connect", lualilka_net_connect},
    {"send", lualilka_net_send},
    {"receive", lualilka_net_receive},
    {"close", lualilka_net_close},
    {"settimeout", lualilka_net_settimeout},
    {"listen", lualilka_net_listen},
    {"accept", lualilka_net_accept},
    {NULL, NULL},
};

int lualilka_socket_register(lua_State* L) {
    luaL_newlib(L, lualilka_net);
    lua_setglobal(L, "net");
    return 0;
}
