#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include "lualilka_http.h"
#include "keira/keira.h"

#define STR_HELPER(x) #x
#define STR(x)        STR_HELPER(x)
const char* defaultUserAgent = "Lilka/" STR(LILKA_VERSION);

static int lualilka_http_execute(lua_State* L) {
    int n = lua_gettop(L);
    if (n != 1) {
        return luaL_error(L, K_S_LUA_HTTP_ARGS_1_FMT, n);
    }

    if (!lua_istable(L, 1)) {
        return luaL_error(L, K_S_LUA_HTTP_ARG_MUST_BE_TABLE);
    }

    const char* url = nullptr;
    const char* method = nullptr;
    const char* body = nullptr;
    const char* fileName = nullptr;

    lua_pushnil(L);
    while (lua_next(L, 1) != 0) {
        if (lua_type(L, -2) == LUA_TSTRING) {
            const char* key = lua_tostring(L, -2);
            if (lua_type(L, -1) == LUA_TSTRING) {
                if (strcmp(key, "url") == 0) {
                    url = lua_tostring(L, -1);
                } else if (strcmp(key, "method") == 0) {
                    method = lua_tostring(L, -1);
                } else if (strcmp(key, "body") == 0) {
                    body = lua_tostring(L, -1);
                } else if (strcmp(key, "file") == 0) {
                    fileName = lua_tostring(L, -1);
                }
            }
        }
        lua_pop(L, 1);
    }

    if (method == nullptr) {
        if (body == nullptr) {
            method = "GET";
        } else {
            method = "POST";
        }
    }

    bool isHttps = url != nullptr && strncmp(url, "https://", 8) == 0;

    HTTPClient http;
    http.setUserAgent(defaultUserAgent);

    WiFiClientSecure secureClient;
    WiFiClient plainClient;

    if (isHttps) {
        secureClient.setInsecure();
        http.begin(secureClient, url);
    } else {
        http.begin(plainClient, url);
    }

    int statusCode;
    if (body == nullptr) {
        statusCode = http.sendRequest(method);
    } else {
        statusCode = http.sendRequest(method, String(body));
    }

    lua_newtable(L);
    lua_pushstring(L, "code");
    lua_pushnumber(L, statusCode);
    lua_settable(L, -3);
    if (statusCode == HTTP_CODE_OK) {
        if (fileName != 0) {
            WiFiClient* stream = http.getStreamPtr();
            FILE* file = fopen(fileName, "wb");
            if (!file) {
                http.end();
                return luaL_error(L, K_S_LUA_HTTP_CANT_OPEN_FILE_FMT, fileName);
            }

            uint8_t buffer[128];
            while (stream->connected() && stream->available()) {
                size_t bytes = stream->readBytes(buffer, sizeof(buffer));
                if (bytes > 0) {
                    fwrite(buffer, 1, bytes, file);
                }
            }
            fclose(file);
        } else {
            String response = http.getString();
            lua_pushstring(L, "response");
            lua_pushstring(L, response.c_str());
            lua_settable(L, -3);
        }
    }

    return 1;
}

static const struct luaL_Reg lualilka_http[] = {
    {"execute", lualilka_http_execute},
    {NULL, NULL},
};

int lualilka_http_register(lua_State* L) {
    luaL_newlib(L, lualilka_http);
    lua_pushnumber(L, HTTP_CODE_OK);
    lua_setfield(L, -2, "HTTP_CODE_OK");
    lua_setglobal(L, "http");

    return 0;
}