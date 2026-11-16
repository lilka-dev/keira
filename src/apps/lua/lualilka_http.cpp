#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <esp_timer.h>
#include <algorithm>
#include <memory>
#include <new>
#include <functional>
#include "lualilka_http.h"
#include "keira/keira.h"
#include "keira/utils/defer.h"

#define STR_HELPER(x) #x
#define STR(x)        STR_HELPER(x)
const char* defaultUserAgent = "Lilka/" STR(LILKA_VERSION);

// Abort the download if no data arrives for this long
static constexpr uint32_t K_LUA_HTTP_STALL_TIMEOUT_MS = 10000;

// Receives the response body piece by piece; returns false if the data could not be stored
using BodySink = std::function<bool(const uint8_t* data, size_t size)>;

// Reads `length` bytes into `sink`, or until the connection closes if `length` is -1.
// Returns 0 on success or a negative HTTPC_ERROR_* code.
static int readBlock(
    WiFiClient* stream, uint8_t* buffer, size_t bufferSize, const BodySink& sink, int64_t length, int64_t& received
) {
    int64_t left = length;
    int64_t lastDataUs = esp_timer_get_time();
    while (left != 0) {
        // Drain buffered data first: it may still be readable after the peer has closed
        int available = stream->available();
        if (available > 0) {
            size_t toRead = std::min(static_cast<size_t>(available), bufferSize);
            if (left > 0) {
                toRead = std::min(toRead, static_cast<size_t>(left));
            }
            int bytesRead = stream->read(buffer, toRead);
            if (bytesRead > 0) {
                if (!sink(buffer, bytesRead)) {
                    return HTTPC_ERROR_STREAM_WRITE;
                }
                received += bytesRead;
                if (left > 0) {
                    left -= bytesRead;
                }
                lastDataUs = esp_timer_get_time();
                continue;
            }
        }
        if (!stream->connected()) {
            // Connection close is the normal end only when the length is unknown
            return left < 0 ? 0 : HTTPC_ERROR_CONNECTION_LOST;
        }
        if (esp_timer_get_time() - lastDataUs > static_cast<int64_t>(K_LUA_HTTP_STALL_TIMEOUT_MS) * 1000) {
            return HTTPC_ERROR_READ_TIMEOUT;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    return 0;
}

// Reads the whole response body into `sink`, verifying it arrived complete.
// Returns 0 on success or a negative HTTPC_ERROR_* code; `received` is the number of body bytes read.
static int readBody(HTTPClient& http, const BodySink& sink, int64_t& received) {
    received = 0;
    WiFiClient* stream = http.getStreamPtr();
    if (stream == nullptr) {
        return HTTPC_ERROR_NO_STREAM;
    }
    // On the heap: app tasks have only a few KB of stack
    const size_t bufferSize = 1436; // one TCP segment
    std::unique_ptr<uint8_t[]> buffer(new (std::nothrow) uint8_t[bufferSize]);
    if (!buffer) {
        return HTTPC_ERROR_TOO_LESS_RAM;
    }
    if (!http.header("Transfer-Encoding").equalsIgnoreCase("chunked")) {
        // getSize() is -1 without Content-Length: the body then ends when the connection closes
        return readBlock(stream, buffer.get(), bufferSize, sink, http.getSize(), received);
    }

    // Chunked: "<hex size>[;ext]\r\n<data>\r\n" repeated, ending with a zero-size chunk
    // Missing framing means the transfer was cut or stalled; only bytes that arrived malformed are a server error
    auto framingError = [stream](bool timedOut) {
        if (!stream->connected()) {
            return HTTPC_ERROR_CONNECTION_LOST;
        }
        return timedOut ? HTTPC_ERROR_READ_TIMEOUT : HTTPC_ERROR_ENCODING;
    };
    while (true) {
        String header = stream->readStringUntil('\n');
        char* end = nullptr;
        int64_t size = strtoll(header.c_str(), &end, 16);
        if (end == header.c_str() || size < 0) {
            return framingError(header.length() == 0);
        }
        if (size == 0) {
            return 0;
        }
        int ret = readBlock(stream, buffer.get(), bufferSize, sink, size, received);
        if (ret < 0) {
            return ret;
        }
        uint8_t crlf[2];
        size_t crlfRead = stream->readBytes(crlf, 2);
        if (crlfRead != 2 || crlf[0] != '\r' || crlf[1] != '\n') {
            return framingError(crlfRead != 2);
        }
    }
}

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

    int statusCode;
    int64_t received = 0;
    bool fileOpenFailed = false;
    String response;
    bool haveResponse = false;
    {
        // Clients must outlive HTTPClient: its end()/destructor calls stop() on them
        WiFiClientSecure secureClient;
        WiFiClient plainClient;
        HTTPClient http;
        Defer endHttp([&http]() { http.end(); });
        http.setUserAgent(defaultUserAgent);
        http.setTimeout(K_LUA_HTTP_STALL_TIMEOUT_MS);
        // One request per call: ask the server to close, so bodies without Content-Length end cleanly
        http.setReuse(false);
        const char* collectedHeaders[] = {"Transfer-Encoding"};
        http.collectHeaders(collectedHeaders, 1);

        if (isHttps) {
            secureClient.setInsecure();
            http.begin(secureClient, url);
        } else {
            http.begin(plainClient, url);
        }

        if (body == nullptr) {
            statusCode = http.sendRequest(method);
        } else {
            statusCode = http.sendRequest(method, String(body));
        }

        if (statusCode == HTTP_CODE_OK) {
            if (fileName != nullptr) {
                FILE* file = fopen(fileName, "wb");
                if (!file) {
                    fileOpenFailed = true;
                } else {
                    int result = readBody(
                        http,
                        [file](const uint8_t* data, size_t size) { return fwrite(data, 1, size, file) == size; },
                        received
                    );
                    // fclose flushes buffered data, so its failure is a write failure too
                    if (fclose(file) != 0 && result >= 0) {
                        result = HTTPC_ERROR_STREAM_WRITE;
                    }
                    if (result < 0) {
                        // Never leave a truncated file behind that looks like a successful download
                        remove(fileName);
                        statusCode = result;
                    }
                }
            } else {
                int result = HTTPC_ERROR_TOO_LESS_RAM;
                // getSize() is -1 without Content-Length; then the String grows as data arrives
                if (http.getSize() <= 0 || response.reserve(http.getSize())) {
                    result = readBody(
                        http,
                        [&response](const uint8_t* data, size_t size) { return response.concat(data, size); },
                        received
                    );
                }
                if (result < 0) {
                    // A partial body is not a valid response
                    response = String();
                    statusCode = result;
                } else {
                    haveResponse = true;
                }
            }
        }
    }

    // Raise Lua errors only after C++ objects are destroyed: luaL_error longjmps past destructors
    if (fileOpenFailed) {
        return luaL_error(L, K_S_LUA_HTTP_CANT_OPEN_FILE_FMT, fileName);
    }

    lua_newtable(L);
    lua_pushnumber(L, statusCode);
    lua_setfield(L, -2, "code");
    lua_pushnumber(L, received);
    lua_setfield(L, -2, "size");
    if (haveResponse) {
        lua_pushlstring(L, response.c_str(), response.length());
        lua_setfield(L, -2, "response");
    }
    if (statusCode < 0) {
        lua_pushstring(L, HTTPClient::errorToString(statusCode).c_str());
        lua_setfield(L, -2, "error");
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