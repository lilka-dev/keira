#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <esp_timer.h>
#include <memory>
#include <new>
#include <algorithm>
#include "lualilka_http.h"
#include "keira/keira.h"
#include "keira/utils/defer.h"

#define STR_HELPER(x) #x
#define STR(x)        STR_HELPER(x)
const char* defaultUserAgent = "Lilka/" STR(LILKA_VERSION);

// Abort the download if no data arrives for this long
static constexpr uint32_t K_LUA_HTTP_STALL_TIMEOUT_MS = 10000;

// Streams the response body into an open file.
// Returns 0 on success or a negative HTTPC_ERROR_* code; `received` is the number of bytes written.
static int downloadToFile(HTTPClient& http, FILE* file, int64_t& received) {
    class FileStream : public Stream {
    public:
        explicit FileStream(FILE* f) : _f(f) {
        }
        size_t write(uint8_t b) override {
            return write(&b, 1);
        }
        size_t write(const uint8_t* buf, size_t size) override {
            size_t written = fwrite(buf, 1, size, _f);
            if (written != size) {
                setWriteError();
            }
            _total += written;
            return written;
        }
        int available() override {
            return 0;
        }
        int read() override {
            return -1;
        }
        int peek() override {
            return -1;
        }
        void flush() override {
        }
        int64_t total() const {
            return _total;
        }

    private:
        FILE* _f;
        int64_t _total = 0;
    };

    if (http.header("Transfer-Encoding").equalsIgnoreCase("chunked")) {
        // HTTPClient handles chunk framing and verifies the total size itself
        FileStream fileStream(file);
        int ret = http.writeToStream(&fileStream);
        received = fileStream.total();
        return ret < 0 ? ret : 0;
    }

    WiFiClient* stream = http.getStreamPtr();
    if (stream == nullptr) {
        return HTTPC_ERROR_NO_STREAM;
    }

    // -1 when the server sent no Content-Length: then the body ends when the connection closes
    const int64_t expected = http.getSize();
    const size_t bufferSize = 1436; // one TCP segment
    std::unique_ptr<uint8_t[]> buffer(new (std::nothrow) uint8_t[bufferSize]);
    if (!buffer) {
        return HTTPC_ERROR_TOO_LESS_RAM;
    }

    int64_t lastDataUs = esp_timer_get_time();
    received = 0;
    while (expected < 0 || received < expected) {
        // Drain buffered data first: it may still be readable after the peer has closed
        int available = stream->available();
        if (available > 0) {
            size_t toRead = std::min(static_cast<size_t>(available), bufferSize);
            if (expected >= 0) {
                toRead = std::min(toRead, static_cast<size_t>(expected - received));
            }
            int bytesRead = stream->read(buffer.get(), toRead);
            if (bytesRead > 0) {
                if (fwrite(buffer.get(), 1, bytesRead, file) != static_cast<size_t>(bytesRead)) {
                    return HTTPC_ERROR_STREAM_WRITE;
                }
                received += bytesRead;
                lastDataUs = esp_timer_get_time();
                continue;
            }
        }
        if (!stream->connected()) {
            break;
        }
        if (esp_timer_get_time() - lastDataUs > static_cast<int64_t>(K_LUA_HTTP_STALL_TIMEOUT_MS) * 1000) {
            return HTTPC_ERROR_READ_TIMEOUT;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    if (expected >= 0 && received != expected) {
        return HTTPC_ERROR_CONNECTION_LOST;
    }
    return 0;
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
                    int result = downloadToFile(http, file, received);
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
                response = http.getString();
                received = response.length();
                haveResponse = true;
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